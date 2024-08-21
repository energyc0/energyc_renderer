#include "EnergycRenderer.h"
#include "MaterialManager.h"
#include "Scene.h"

const std::string sphere_filename = std::string(RENDERER_DIRECTORY) + "/assets/sphere.obj";
const std::string cube_filename = std::string(RENDERER_DIRECTORY) + "/assets/cube.obj";

EnergycRenderer::EnergycRenderer(int width, int height, const char* application_name, const char* engine_name) :
	_window(width, height, application_name),
	_core(_window.get_window(), application_name, engine_name),
	_camera(glm::vec3(0.0f, 0.f, -10.f)),
	_controller(_window, _camera),
	_material_manager(new MaterialManager()),
	_current_scene(std::shared_ptr<Scene>(new Scene(_material_manager))),
	_gui_info(0.f,*_current_scene),
	_scenes{ _current_scene } {

	ObjectMaterial rusted_iron = _material_manager->create_new_material(
		"Rusted iron",
		(std::string(RENDERER_DIRECTORY) + "/assets/bricks.jpg").c_str(),
		(std::string(RENDERER_DIRECTORY) + "/assets/rustediron2_metallic.png").c_str(),
		(std::string(RENDERER_DIRECTORY) + "/assets/rustediron2_roughness.png").c_str(),
		(std::string(RENDERER_DIRECTORY) + "/assets/bricks_norm.jpg").c_str());

	std::shared_ptr<Mesh> sphere1(new Mesh(sphere_filename.c_str()));
	std::shared_ptr<Mesh> sphere2(new Mesh(*sphere1));
	std::shared_ptr<Mesh> cube(new Mesh(cube_filename.c_str()));
	std::shared_ptr<PointLight> light1(
		new PointLight("My point light", glm::vec3(0.8,1.3,1.2), glm::vec3(1.f), 0.1));
	
	sphere1->set_pos(glm::vec3(10.f));
	sphere1->set_size(glm::vec3(3.f));
	sphere1->set_material(rusted_iron);

	sphere2->set_pos(glm::vec3(- 10.f, 0.f, 0.f));
	sphere2->set_size(glm::vec3(1.f));
	sphere2->set_material(rusted_iron);

	cube->set_pos(glm::vec3(1.f,-2,1.f));
	cube->set_size(glm::vec3(3.f));
	cube->set_material(rusted_iron);

	_current_scene->add_mesh(sphere1);
	_current_scene->add_mesh(sphere2);
	_current_scene->add_point_light(light1);
	_current_scene->add_mesh(cube);

	RenderManagerCreateInfo render_manager_create_info{
		_current_scene,
		_controller.get_camera(),
		_window,
		_gui_info,
		_material_manager
	};
	_render_manager = std::unique_ptr<RenderManager>(new RenderManager(render_manager_create_info));

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

void EnergycRenderer::update_uniform(float delta_time) {
	_gui_info.delta_time = delta_time;

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

	update_uniform(delta_time);
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
}