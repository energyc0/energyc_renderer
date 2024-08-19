#include "Scene.h"
#include "CommandManager.h"

constexpr VkDeviceSize VERTEX_BUFFER_ALLOCATION_SIZE = 100000 * sizeof(Vertex);
constexpr VkDeviceSize INDEX_BUFFER_ALLOCATION_SIZE = 500000 * sizeof(uint32_t);
constexpr uint32_t GROUP_MODEL_LIMIT = 30;

Scene::Scene(const std::vector<SceneObject*>& objects) noexcept {

	auto bindings = Model::get_bindings();
	VkDescriptorSetLayoutCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	create_info.bindingCount = bindings.size();
	create_info.pBindings = bindings.data();
	VK_ASSERT(vkCreateDescriptorSetLayout(Core::get_device(), &create_info, nullptr, &_model_group_layout), "vkCreateDescriptorSetLayout(), RendererSolid - FAILED");

	for (const auto& i : objects) {
		add_object(i);
	}
	LOG_STATUS("Created new scene.");
}

void Scene::draw(VkCommandBuffer command_buffer, VkPipelineLayout layout) const noexcept{
	for (auto& group : _object_groups) {
		group->draw(command_buffer,layout);
	}
}

void Scene::add_mesh(Mesh* mesh) {
	bool has_added = false;
	for (auto& group : _object_groups) {
		if (group->try_add_mesh(mesh)) {
			has_added = true;
			break;
		}
	}

	if (!has_added) {
		_object_groups.push_back(new ModelGroup(mesh, _model_group_layout));
	}
}

void Scene::add_object(SceneObject* object) {
	_objects.push_back(object);
	if (dynamic_cast<Mesh*>(object) != nullptr) {
		add_mesh(static_cast<Mesh*>(object));
	}
}

Scene::~Scene() {
	vkDestroyDescriptorSetLayout(Core::get_device(), _model_group_layout, nullptr);
	for (SceneObject* i : _objects) {
		delete i;
	}
	for (ModelGroup* group : _object_groups) {
		delete group;
	}
}

void Scene::ModelGroup::create_descriptor_tools(VkDescriptorSetLayout layout) {
	uint32_t image_count = Core::get_swapchain_image_count();
	VkDescriptorPoolSize pool_sizes[1];
	pool_sizes[0].descriptorCount = image_count;
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	VkDescriptorPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.pPoolSizes = pool_sizes;
	create_info.poolSizeCount = 1;
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
	write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	write.dstArrayElement = 0;
	write.dstBinding = 1;
	for (uint32_t i = 0; i < image_count; i++) {
		VkDescriptorBufferInfo info = _storage_buffers[i]->get_info(0, VK_WHOLE_SIZE);
		write.dstSet = _descriptor_sets[i];
		write.pBufferInfo = &info;

		write_descriptors.push_back(write);
	}
	vkUpdateDescriptorSets(Core::get_device(), write_descriptors.size(), write_descriptors.data(), 0, 0);
}

void Scene::ModelGroup::create_buffers(Mesh* object) {
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

void Scene::ModelGroup::push_model(const Mesh* mesh) {
	const VkDeviceSize vertex_size = mesh->get_vertices_count() * sizeof(Vertex);
	const VkDeviceSize index_size = mesh->get_indices_count() * sizeof(uint32_t);

	VulkanBuffer staging_vertex_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		vertex_size,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VulkanBuffer staging_index_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		index_size,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	char* ptr = staging_index_buffer.map_memory(0, index_size);
	memcpy(ptr, mesh->get_index_data(), index_size);
	staging_index_buffer.unmap_memory();

	ptr = staging_vertex_buffer.map_memory(0, vertex_size);
	memcpy(ptr, mesh->get_vertex_data(), vertex_size);
	staging_vertex_buffer.unmap_memory();

	VkCommandBuffer cmd = CommandManager::begin_single_command_buffer();
	CommandManager::copy_buffers(cmd, staging_vertex_buffer, *_vertex_buffer, 0, sizeof(Vertex) * _total_vertices, staging_vertex_buffer.get_size());
	CommandManager::copy_buffers(cmd, staging_index_buffer, *_index_buffer, 0, sizeof(uint32_t) * _total_indices, staging_index_buffer.get_size());
	CommandManager::end_single_command_buffer(cmd);

	std::vector<void*> ptrs;
	ptrs.reserve(_buffer_data_ptrs.size());
	for (char*& ptr : _buffer_data_ptrs) {
		ptrs.push_back((void*)ptr);
		ptr += sizeof(glm::mat4);
	}
	_models.push_back(new Model(mesh, _models.size(), _total_vertices, _total_indices, ptrs));

	_total_vertices += mesh->get_vertices_count();
	_total_indices += mesh->get_indices_count();
	_empty_indices -= mesh->get_indices_count() * sizeof(uint32_t);
	_empty_vertices -= mesh->get_vertices_count() * sizeof(Vertex);
	_storage_buffer_space -= _descriptor_sets.size();
	LOG_STATUS("Created model.");
}

Scene::ModelGroup::ModelGroup(Mesh* object, VkDescriptorSetLayout layout) noexcept :
	_total_indices(0),
	_total_vertices(0),
	_storage_buffer_space(GROUP_MODEL_LIMIT) {
	create_descriptor_tools(layout);
	create_buffers(object);
	_empty_indices = _index_buffer->get_size();
	_empty_vertices = _vertex_buffer->get_size();
	push_model(object);
	LOG_STATUS("Created new ModelGroup.");
}

bool Scene::ModelGroup::try_add_mesh(const Mesh* mesh) {
	if (mesh->get_indices_count() * sizeof(uint32_t) > _empty_indices ||
		mesh->get_vertices_count() * sizeof(Vertex) > _empty_vertices || 
		_storage_buffer_space < _descriptor_sets.size()) {
		return false;
	}
	push_model(mesh);
	return true;
}

void Scene::ModelGroup::draw(VkCommandBuffer command_buffer, VkPipelineLayout layout) {
	_index_buffer->bind_index_buffer(command_buffer, 0);
	_vertex_buffer->bind_vertex_buffer(command_buffer, 0);
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1, &_descriptor_sets[Core::get_current_frame()], 0, 0);
	for (auto& obj : _models) {
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