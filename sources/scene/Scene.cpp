#include "Scene.h"
#include "imgui.h"
#include "CommandManager.h"
#include "MaterialManager.h"

constexpr VkDeviceSize VERTEX_BUFFER_ALLOCATION_SIZE = 500000 * sizeof(Vertex);
constexpr VkDeviceSize INDEX_BUFFER_ALLOCATION_SIZE = 500000 * sizeof(uint32_t);
constexpr uint32_t GROUP_MODEL_LIMIT = 30;
constexpr uint32_t POINT_LIGHT_LIMIT = 10;

Scene::Scene(const std::shared_ptr<MaterialManager>& material_manager) noexcept :
	_material_manager(material_manager) {
	create_buffers();
	create_descriptor_tools();
	LOG_STATUS("Created new scene.");
}
void Scene::create_buffers() {
	uint32_t image_count = Core::get_swapchain_image_count();
	_scene_lights_buffers.reserve(image_count);
	for (uint32_t i = 0; i < image_count; i++) {
		_scene_lights_buffers.push_back(new VulkanBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			sizeof(PointLightData) * POINT_LIGHT_LIMIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
	}
}
void Scene::draw_solid(VkCommandBuffer command_buffer, VkPipelineLayout layout) const noexcept{
	for (auto& group : _object_groups) {
		group->draw(command_buffer,layout, _material_manager);
	}
}

void Scene::draw_light(VkCommandBuffer command_buffer, VkPipelineLayout layout) {
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		layout, 1, 1, &_point_light_descriptor_sets[Core::get_current_frame()], 0, 0);
	for (uint32_t i = 0; i < _point_lights.size(); i++) {
		if (!_point_lights[i]->is_copied()) {
			_point_lights[i]->set_copied();
			copy_light_new_info(_point_lights[i], i);
		}
	}
	vkCmdDraw(command_buffer, _point_lights.size() * 6, _point_lights.size(), 0, 0);
}

void Scene::display_scene_info_gui(bool* is_window_opened) const noexcept {
	ImGui::BeginChild("Scene info: ", ImVec2(0.f,0.f),
		ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY| ImGuiChildFlags_AlwaysAutoResize,
		ImGuiWindowFlags_NoResize);
	for (SceneObject* object : _objects) {
		object->display_gui_info();
	}
	ImGui::EndChild();
}

void Scene::create_descriptor_tools() {
	auto bindings = PointLight::get_bindings();
	auto model_bindings = Model::get_bindings();
	for (auto& i : model_bindings) {
		bindings.push_back(i);
	}
	VkDescriptorSetLayoutCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	create_info.bindingCount = bindings.size();
	create_info.pBindings = bindings.data();
	VK_ASSERT(vkCreateDescriptorSetLayout(Core::get_device(), &create_info, nullptr, &_descriptor_set_layout), "vkCreateDescriptorSetLayout(), RendererSolid - FAILED");
}

void Scene::copy_light_new_info(const std::shared_ptr<PointLight>& light, uint32_t idx) noexcept {
	PointLightData data = light->get_data();
	StagingBuffer::copy_data_to_buffer(&data, sizeof(PointLightData));

	auto cmd = CommandManager::begin_single_command_buffer();

	StagingBuffer::copy_buffers(cmd, *_scene_lights_buffers[Core::get_current_frame()],
		0, idx * sizeof(PointLight));

	CommandManager::end_single_command_buffer(cmd);
}

bool Scene::add_mesh(const std::shared_ptr<Mesh>& mesh) {
	bool has_added = false;
	for (auto& group : _object_groups) {
		if (group->try_add_mesh(mesh)) {
			has_added = true;
			_objects.push_back(group->get_last_pushed_model());
			break;
		}
	}

	if (!has_added) {
		_object_groups.push_back(new ModelGroup(mesh, _descriptor_set_layout, _scene_lights_buffers));
		_objects.push_back(_object_groups.back()->get_last_pushed_model());
		if (_point_light_descriptor_sets.empty()) {
			_point_light_descriptor_sets = _object_groups.back()->get_descriptor_sets();
		}
	}
	return true;
}

bool Scene::add_point_light(const std::shared_ptr<PointLight>& light) {
	if (_point_lights.size() >= POINT_LIGHT_LIMIT) {
		LOG_STATUS("Point light limit exceded. Aborted adding the light.");
		return false;
	}
	PointLightData data = light->get_data();
	StagingBuffer::copy_data_to_buffer(&data, sizeof(PointLightData));

	auto cmd = CommandManager::begin_single_command_buffer();

	for (auto& buffer : _scene_lights_buffers) {
		StagingBuffer::copy_buffers(cmd, *buffer,
			0, _point_lights.size() * sizeof(PointLightData));
	}

	CommandManager::end_single_command_buffer(cmd);

	_point_lights.push_back(light);
	_objects.push_back(light.get());
	LOG_STATUS("Added point light: ", light->get_name());
	return true;
}

Scene::~Scene() {
	vkDestroyDescriptorSetLayout(Core::get_device(), _descriptor_set_layout, nullptr);
	for (VulkanBuffer* buffer : _scene_lights_buffers) {
		delete buffer;
	}
	for (ModelGroup* group : _object_groups) {
		delete group;
	}
}

void Scene::ModelGroup::create_descriptor_tools(VkDescriptorSetLayout layout, const std::vector<VulkanBuffer*>& scene_lights_buffers) {
	uint32_t image_count = Core::get_swapchain_image_count();
	VkDescriptorPoolSize pool_sizes[2];
	pool_sizes[0].descriptorCount = image_count;
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pool_sizes[1].descriptorCount = image_count;
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.pPoolSizes = pool_sizes;
	create_info.poolSizeCount = 2;
	create_info.maxSets = image_count;
	VK_ASSERT(vkCreateDescriptorPool(Core::get_device(), &create_info, nullptr, &_descriptor_pool), "vkCreateDescriptorPool(), RendererSolid - FAILED");

	_storage_buffers.resize(image_count);
	_descriptor_sets.resize(image_count);
	_buffer_data_ptrs.reserve(image_count);
	for (auto& buffer : _storage_buffers) {
		buffer = new VulkanBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			sizeof(glm::mat4) * GROUP_MODEL_LIMIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		_buffer_data_ptrs.push_back(buffer->map_memory(0, VK_WHOLE_SIZE));
	}

	std::vector< VkDescriptorSetLayout> layouts(image_count,layout);
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = _descriptor_pool;
	alloc_info.descriptorSetCount = image_count;
	alloc_info.pSetLayouts = layouts.data();
	VK_ASSERT(vkAllocateDescriptorSets(Core::get_device(), &alloc_info, _descriptor_sets.data()), "vkAllocateDescriptorSets() - FAILED");

	std::vector<VkWriteDescriptorSet> write_descriptors;
	write_descriptors.reserve(image_count);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorCount = 1;
	write.dstArrayElement = 0;
	for (uint32_t i = 0; i < image_count; i++) {
		write.dstSet = _descriptor_sets[i];

		VkDescriptorBufferInfo storage_buffer_info = _storage_buffers[i]->get_info(0, VK_WHOLE_SIZE);
		write.dstBinding = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write.pBufferInfo = &storage_buffer_info;
		write_descriptors.push_back(write);

		VkDescriptorBufferInfo point_lights_buffer_info = scene_lights_buffers[i]->get_info(0, VK_WHOLE_SIZE);
		write.dstBinding = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.pBufferInfo = &point_lights_buffer_info;
		write_descriptors.push_back(write);
	}
	vkUpdateDescriptorSets(Core::get_device(), write_descriptors.size(), write_descriptors.data(), 0, 0);
}

void Scene::ModelGroup::create_buffers(const std::shared_ptr<Mesh>& object) {
	const VkDeviceSize vertex_size = object->get_vertices_count() * sizeof(Vertex);
	const VkDeviceSize index_size = object->get_indices_count() * sizeof(uint32_t);

	if (vertex_size < VERTEX_BUFFER_ALLOCATION_SIZE && index_size < INDEX_BUFFER_ALLOCATION_SIZE) {
		_index_buffer = new VulkanBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			INDEX_BUFFER_ALLOCATION_SIZE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		_vertex_buffer = new VulkanBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VERTEX_BUFFER_ALLOCATION_SIZE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}
	else {
		_index_buffer = new VulkanBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			index_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		_vertex_buffer = new VulkanBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			vertex_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}
}

void Scene::ModelGroup::push_model(const std::shared_ptr<Mesh>& mesh) {
	const VkDeviceSize vertex_size = mesh->get_vertices_count() * sizeof(Vertex);
	const VkDeviceSize index_size = mesh->get_indices_count() * sizeof(uint32_t);

	StagingBuffer::copy_data_to_buffer(mesh->get_index_data(), index_size);

	VkCommandBuffer cmd = CommandManager::begin_single_command_buffer();
	StagingBuffer::copy_buffers(cmd, *_index_buffer, 0, sizeof(uint32_t) * _total_indices);
	CommandManager::end_single_command_buffer(cmd);

	StagingBuffer::copy_data_to_buffer(mesh->get_vertex_data(), vertex_size);

	cmd = CommandManager::begin_single_command_buffer();
	StagingBuffer::copy_buffers(cmd, *_vertex_buffer, 0, sizeof(Vertex) * _total_vertices);
	CommandManager::end_single_command_buffer(cmd);

	std::vector<void*> ptrs;
	ptrs.reserve(_buffer_data_ptrs.size());
	for (char*& ptr : _buffer_data_ptrs) {
		ptrs.push_back((void*)ptr);
		ptr += sizeof(glm::mat4);
	}
	_last_pushed_model = new Model(mesh.get(), _models.size(), _total_vertices, _total_indices, ptrs);
	_models.push_back(_last_pushed_model);

	_total_vertices += mesh->get_vertices_count();
	_total_indices += mesh->get_indices_count();
	_empty_indices -= mesh->get_indices_count() * sizeof(uint32_t);
	_empty_vertices -= mesh->get_vertices_count() * sizeof(Vertex);
	_storage_buffer_space -= _descriptor_sets.size();

	LOG_STATUS("Added model: ", mesh->get_name());
}

Scene::ModelGroup::ModelGroup(const std::shared_ptr<Mesh>& object, VkDescriptorSetLayout layout, const std::vector<VulkanBuffer*>& scene_lights_buffers) noexcept :
	_total_indices(0),
	_total_vertices(0),
	_storage_buffer_space(GROUP_MODEL_LIMIT) {

	create_descriptor_tools(layout, scene_lights_buffers);
	create_buffers(object);
	_empty_indices = _index_buffer->get_size();
	_empty_vertices = _vertex_buffer->get_size();
	push_model(object);

	LOG_STATUS("Created new ModelGroup.");
}

bool Scene::ModelGroup::try_add_mesh(const std::shared_ptr<Mesh>& mesh) {
	if (mesh->get_indices_count() * sizeof(uint32_t) > _empty_indices ||
		mesh->get_vertices_count() * sizeof(Vertex) > _empty_vertices || 
		_storage_buffer_space < _descriptor_sets.size()) {
		return false;
	}
	push_model(mesh);
	return true;
}

void Scene::ModelGroup::draw(VkCommandBuffer command_buffer, VkPipelineLayout layout, const std::shared_ptr<MaterialManager>& material_manager) {
	_index_buffer->bind_index_buffer(command_buffer, 0);
	_vertex_buffer->bind_vertex_buffer(command_buffer, 0);
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1, &_descriptor_sets[Core::get_current_frame()], 0, 0);
	int32_t prev_material_idx = INT32_MIN;
	for (auto& obj : _models) {
		if (prev_material_idx != obj->get_material_index()) {
			VkDescriptorSet set = material_manager->get_material_descriptor(obj);
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 2, 1, &set, 0, 0);
		}
		obj->draw(command_buffer);
	}
}

Scene::ModelGroup::~ModelGroup() {
	vkDestroyDescriptorPool(Core::get_device(), _descriptor_pool, nullptr);
	for (auto& buffer : _storage_buffers) {
		delete buffer;
	}
	for (auto& model : _models) {
		delete model;
	}
	delete _vertex_buffer;
	delete _index_buffer;
}