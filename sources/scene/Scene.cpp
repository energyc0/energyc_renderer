#include "Scene.h"
#include "CommandManager.h"

constexpr VkDeviceSize VERTEX_BUFFER_ALLOCATION_SIZE = 10000 * sizeof(Vertex);
constexpr VkDeviceSize INDEX_BUFFER_ALLOCATION_SIZE = 50000 * sizeof(uint32_t);

Scene::Scene(const std::vector<SceneObject*>& objects) {
	for (const auto& i : objects) {
		add_object(i);
	}
}

void Scene::draw(VkCommandBuffer command_buffer) {
	for (auto& group : _object_groups) {
		group->draw(command_buffer);
	}
}

void Scene::add_object(SceneObject* object) {
	_objects.push_back(object);
	if (dynamic_cast<Model*>(object) != nullptr) {
		bool has_added = false;
		Model* model = static_cast<Model*>(object);

		for (auto& group : _object_groups) {
			if (group->try_add_model(model)) {
				has_added = true;
				break;
			}
		}

		if (!has_added) {
			_object_groups.push_back(new ModelGroup(model));
		}
	}
}

Scene::~Scene() {
	for (SceneObject* i : _objects) {
		delete i;
	}
	for (ModelGroup* group : _object_groups) {
		delete group;
	}
}

Scene::ModelGroup::ModelGroup(Model* object) {
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
	VulkanBuffer staging_vertex_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		vertex_size,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VulkanBuffer staging_index_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		index_size,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	char* ptr = staging_index_buffer.map_memory(0, index_size);
	memcpy(ptr, object->get_index_data(), index_size);
	staging_index_buffer.unmap_memory();

	ptr = staging_vertex_buffer.map_memory(0, vertex_size);
	memcpy(ptr, object->get_vertex_data(), vertex_size);
	staging_vertex_buffer.unmap_memory();

	VkCommandBuffer cmd = CommandManager::begin_single_command_buffer();
	VulkanBuffer::copy_buffers(cmd, staging_vertex_buffer, *_vertex_buffer, 0, 0, staging_vertex_buffer.get_size());
	VulkanBuffer::copy_buffers(cmd, staging_index_buffer, *_index_buffer, 0, 0, staging_index_buffer.get_size());
	CommandManager::end_single_command_buffer(cmd);

	_render_objects.push_back(object);
}

bool Scene::ModelGroup::try_add_model(Model* object) {
	return false;
}

void Scene::ModelGroup::draw(VkCommandBuffer command_buffer) {
	_index_buffer->bind_index_buffer(command_buffer, 0);
	_vertex_buffer->bind_vertex_buffer(command_buffer, 0);
	for (auto& obj : _render_objects) {
		obj->draw(command_buffer);
	}
}

Scene::ModelGroup::~ModelGroup() {
	delete _vertex_buffer;
	delete _index_buffer;
}