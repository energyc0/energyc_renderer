#include "RendererApplication.h"
#include "Scene.h"

RendererApplication::RendererApplication(int width, int height, const char* application_name, const char* engine_name) :
	_window(width, height, application_name),
	_core(_window.get_window(), application_name, engine_name),
	//_camera(glm::vec3(0.0f), glm::vec3(1.f)),
	_controller(_window, _camera){
	_scenes.push_back(new Scene({ new Model(
		{{glm::vec3(2.5, 2.5, 1.0) ,glm::vec3(1.0,0.0,0.0),glm::vec3(0.0)},
		{glm::vec3(-2.5, 2.5, 1.0), glm::vec3(0.0,1.0,0.0)},
		{glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0,0.0,1.0)}},
		{0,1,2}
)}));
	_render_unit_solid = new RenderUnitSolid(*_scenes[0], _controller.get_camera());
	LOG_STATUS("Application start.");
}

void RendererApplication::run() {
	while (!glfwWindowShouldClose(_window.get_window())) {
		glfwPollEvents();
		float delta_time = _timer.process_time();
		_controller.process_input(delta_time);
		render(delta_time);
		_core.next_frame();
	}
}

void RendererApplication::update_uniform() {
	_render_unit_solid->update_descriptor_sets();
}

void RendererApplication::update_render_tasks(float delta_time) {
	_command_manager.begin_frame_command_buffer();

	_render_unit_solid->fill_command_buffer(_command_manager.get_frame_command_buffer());

	_command_manager.end_frame_command_buffer();
}

void RendererApplication::render(float delta_time) {
	VkFence fence = _sync_manager.get_fence();
	vkWaitForFences(_core.get_device(), 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(_core.get_device(), 1, &fence);

	VkSemaphore semaphore_to_render = _sync_manager.get_semaphore_to_render();
	auto result = _core.acquire_next_image(semaphore_to_render,NULL);

	update_uniform();
	update_render_tasks(delta_time);

	VkSemaphore semaphore_to_present = _sync_manager.get_semaphore_to_present();
	VK_ASSERT(CommandManager::submit_queue(
		{ _command_manager.get_frame_command_buffer() },	//command buffers
		{ semaphore_to_render },							//wait semaphores
		{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },	//wait stage flags
		{ semaphore_to_present },							//signal semaphores
		fence),												//fence
		"vkQueueSubmit() - FAILED");

	result = _core.queue_present({ semaphore_to_present });
}

RendererApplication::~RendererApplication() {
	LOG_STATUS("Application shutdown.");
	vkDeviceWaitIdle(Core::get_device());
	for (auto scene : _scenes) {
		delete scene;
	}
	delete _render_unit_solid;
}