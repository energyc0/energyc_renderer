#include "EnergycRenderer.h"
#include "MaterialManager.h"
#include "Scene.h"

const std::string sphere_filename = std::string(RENDERER_DIRECTORY) + "/assets/sphere.obj";
const std::string cube_filename = std::string(RENDERER_DIRECTORY) + "/assets/cube.obj";

const std::string rusted_iron_albedo_filename = std::string(RENDERER_DIRECTORY) + "/assets/rustediron2_basecolor.png";
const std::string rusted_iron_metallic_filename = std::string(RENDERER_DIRECTORY) + "/assets/rustediron2_metallic.png";
const std::string rusted_iron_roughness_filename = std::string(RENDERER_DIRECTORY) + "/assets/rustediron2_roughness.png";
const std::string rusted_iron_normal_filename = std::string(RENDERER_DIRECTORY) + "/assets/rustediron2_normal.png";

EnergycRenderer::EnergycRenderer(int width, int height, const char* application_name, const char* engine_name) :
	_window(width, height, application_name),
	_core(_window.get_window(), application_name, engine_name),
	_camera(glm::vec3(0.0f, 0.f, -5.f)),
	_controller(_window, _camera),
	_material_manager(new MaterialManager()),
	_current_scene(std::shared_ptr<Scene>(new Scene(_material_manager))),
	_gui_info(0.f,*_current_scene, _material_manager),
	_scenes{ _current_scene } {

	std::shared_ptr<PointLight> light(
		new PointLight("My point light", glm::vec3(10.f), glm::vec3(1.f), 0.1));
	
	auto rusted_iron = _material_manager->create_new_material("Rusted iron",
		rusted_iron_albedo_filename.c_str(),
		rusted_iron_metallic_filename.c_str(),
		rusted_iron_roughness_filename.c_str(),
		rusted_iron_normal_filename.c_str());

	_current_scene->add_point_light(light);

	std::shared_ptr<Mesh> sphere(new Mesh(sphere_filename.c_str()));
	sphere->set_material(rusted_iron);
	for (float x = -6.f; x < 6.f; x += 2.f) {
		for (float y = -6.f; y < 6.f; y += 2.f) {
			sphere->set_pos(glm::vec3(x,y,5.f));
			_current_scene->add_mesh(sphere);
		}
	}
	//_current_scene->add_mesh(sphere);
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

	VkCommandBuffer command_buffer = _command_manager.get_frame_command_buffer();

	//if you need to update uniform buffer and the previous frame is writing
	CommandManager::set_memory_dependency(command_buffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);

	_render_manager->update_descriptor_sets(command_buffer);

	CommandManager::set_memory_dependency(command_buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_UNIFORM_READ_BIT);
}

void EnergycRenderer::update_render_tasks(float delta_time) {
	VkCommandBuffer command_buffer = _command_manager.get_frame_command_buffer();
	_render_manager->render(command_buffer);

	_command_manager.end_frame_command_buffer();
}

void EnergycRenderer::draw_frame(float delta_time) {
	CurrentFrameSync frame_sync = _sync_manager.get_current_frame_sync_objects();

	vkWaitForFences(_core.get_device(), 1, &frame_sync.fence, VK_TRUE, UINT64_MAX);
	vkResetFences(_core.get_device(), 1, &frame_sync.fence);

	auto result = _core.acquire_next_image(frame_sync.semaphore_to_render,NULL);

	_command_manager.begin_frame_command_buffer();
	update_uniform(delta_time);
	update_render_tasks(delta_time);

	result = CommandManager::submit_queue(
		{ _command_manager.get_frame_command_buffer() },	//command buffers
		frame_sync.wait_semaphores,							//wait semaphores
		frame_sync.wait_submit_flags,						//wait stage flags
		frame_sync.signal_submit_semaphores,				//signal semaphores
		frame_sync.fence);									//fence
	VK_ASSERT(result, "vkQueueSubmit() - FAILED");

	result = _core.queue_present(frame_sync.present_image_semaphores);
}

EnergycRenderer::~EnergycRenderer() {
	LOG_STATUS("Application shutdown.");
	vkDeviceWaitIdle(Core::get_device());
}