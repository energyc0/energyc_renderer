#include "EnergycRenderer.h"
#include "Scene.h"

const std::string sphere_filename = std::string(RENDERER_DIRECTORY) + "/assets/sphere.obj";

EnergycRenderer::EnergycRenderer(int width, int height, const char* application_name, const char* engine_name) :
	_window(width, height, application_name),
	_core(_window.get_window(), application_name, engine_name),
	_camera(glm::vec3(0.0f, 0.f, -10.f)),
	_controller(_window, _camera) {
	Mesh* sphere1 = new Mesh(sphere_filename.c_str());
	Mesh* sphere2 = new Mesh(sphere_filename.c_str());

	sphere1->set_pos(glm::vec3(10.f));
	sphere1->set_size(glm::vec3(3.f));

	sphere2->set_pos(glm::vec3(- 10.f, 0.f, 0.f));
	sphere2->set_size(glm::vec3(1.f));

	Scene* scene = new Scene({});
	scene->add_object(sphere1);
	scene->add_object(sphere2);

	_scenes.push_back(scene);
	_render_manager = new RenderManager(*_scenes[0], _controller.get_camera());
	LOG_STATUS("Application start.");
}

void EnergycRenderer::run() {
	while (!glfwWindowShouldClose(_window.get_window())) {
		glfwPollEvents();
		float delta_time = _timer.process_time();
		_controller.process_input(delta_time);
		draw_frame(delta_time);
		_core.next_frame();
	}
}

void EnergycRenderer::update_uniform() {
	_render_manager->update_descriptor_sets();
}

void EnergycRenderer::update_render_tasks(float delta_time) {
	_command_manager.begin_frame_command_buffer();

	_render_manager->render(_command_manager.get_frame_command_buffer());

	_command_manager.end_frame_command_buffer();
}

void EnergycRenderer::draw_frame(float delta_time) {
	VkFence fence = _sync_manager.get_fence();
	vkWaitForFences(_core.get_device(), 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(_core.get_device(), 1, &fence);

	VkSemaphore semaphore_to_render = _sync_manager.get_semaphore_to_render();
	VkSemaphore semaphore_to_present = _sync_manager.get_semaphore_to_present();
	auto result = _core.acquire_next_image(semaphore_to_render,NULL);

	update_uniform();
	update_render_tasks(delta_time);

	result = CommandManager::submit_queue(
		{ _command_manager.get_frame_command_buffer() },	//command buffers
		{ semaphore_to_render },							//wait semaphores
		{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },	//wait stage flags
		{ semaphore_to_present },							//signal semaphores
		fence);												//fence
	VK_ASSERT(result, "vkQueueSubmit() - FAILED");

	result = _core.queue_present({ semaphore_to_present });
}

EnergycRenderer::~EnergycRenderer() {
	LOG_STATUS("Application shutdown.");
	vkDeviceWaitIdle(Core::get_device());
	for (auto scene : _scenes) {
		delete scene;
	}
	delete _render_manager;
}