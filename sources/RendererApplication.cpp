#include "RendererApplication.h"

RendererApplication::RendererApplication(int width, int height, const char* application_name, const char* engine_name) :
	_window(width,height,application_name),
	_core(_window.get_window(), application_name,engine_name){
	LOG_STATUS("Application start.");
}

void RendererApplication::run() {
	while (!glfwWindowShouldClose(_window.get_window())) {
		glfwPollEvents();
		render();
		_core.next_frame();
	}
}

void RendererApplication::update_render_tasks() {
	_command_manager.begin_frame_command_buffer();

	_render_unit_solid.fill_command_buffer(_command_manager.get_frame_command_buffer());

	_command_manager.end_frame_command_buffer();
}

void RendererApplication::render() {
	VkFence fence = _sync_manager.get_fence();
	vkWaitForFences(_core.get_device(), 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(_core.get_device(), 1, &fence);

	VkSemaphore semaphore_to_render = _sync_manager.get_semaphore_to_render();
	auto result = _core.acquire_next_image(semaphore_to_render,NULL);

	update_render_tasks();

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
}