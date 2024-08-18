#include "SceneObject.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

std::vector<VkVertexInputAttributeDescription> Vertex::get_attribute_description() {
	std::vector<VkVertexInputAttributeDescription> descriptions(3);
	descriptions[0].binding = 0;
	descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	descriptions[0].location = 0;
	descriptions[0].offset = offsetof(Vertex, pos);

	descriptions[1].binding = 0;
	descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	descriptions[1].location = 1;
	descriptions[1].offset = offsetof(Vertex, color);

	descriptions[2].binding = 0;
	descriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	descriptions[2].location = 2;
	descriptions[2].offset = offsetof(Vertex, normal);

	return descriptions;
}

std::vector<VkVertexInputBindingDescription> Vertex::get_binding_description() {
	std::vector<VkVertexInputBindingDescription> bindings(1);

	bindings[0].binding = 0;
	bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	bindings[0].stride = sizeof(Vertex);

	return bindings;
}

PositionedObject::PositionedObject() noexcept : _world_pos(0.f){}
PositionedObject::PositionedObject(const glm::vec3& pos) noexcept  : _world_pos(pos) {}

void PositionedObject::set_pos(const glm::vec3& pos) noexcept {
	_world_pos = pos;
}

Mesh Mesh::load_mesh(const char* filename) noexcept {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename)) {
		LOG_ERROR(warn, '\n', err);
	}
	
	Mesh mesh;
	uint32_t current_index = 0;
	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};
			vertex.pos.x = attrib.vertices[3 * index.vertex_index + 0];
			vertex.pos.y = attrib.vertices[3 * index.vertex_index + 1];
			vertex.pos.z = attrib.vertices[3 * index.vertex_index + 2];

			vertex.color = glm::vec3(1.0f);

			vertex.normal.x = attrib.normals[3 * index.normal_index + 0];
			vertex.normal.y = attrib.normals[3 * index.normal_index + 1];
			vertex.normal.z = attrib.normals[3 * index.normal_index + 2];

			mesh._vertices.push_back(vertex);
			mesh._indices.push_back(current_index++);
		}
	}

	return mesh;
}

WorldObject::WorldObject(glm::vec3 world_pos,glm::vec3 size,glm::quat rotation):
	PositionedObject(world_pos), _size(size), _rotation(rotation){}

void WorldObject::set_size(const glm::vec3& size) noexcept {
	_size = size;
}

void WorldObject::set_rotation(const glm::quat& rotation) noexcept {
	_rotation = rotation;
}

Mesh::Mesh() noexcept : _vertices(), _indices() {}
Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
	glm::vec3 world_pos,glm::vec3 size,	glm::quat rotation) noexcept :
	WorldObject(world_pos,size,rotation), _vertices(vertices), _indices(indices) {}

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices,
	glm::vec3 world_pos,
	glm::vec3 size,
	glm::quat rotation) noexcept :
	WorldObject(world_pos, size, rotation),  _vertices(std::move(vertices)), _indices(std::move(indices)) {}

Mesh::Mesh(const char* filename,
	glm::vec3 world_pos,
	glm::vec3 size,
	glm::quat rotation) noexcept :
	Mesh(load_mesh(filename)) {
	_world_pos = world_pos;
	_size = size;
	_rotation = rotation;
}


void Model::set_size(const glm::vec3& size) noexcept {
	_size = size;
	_is_transformed = false;
}

void Model::set_rotation(const glm::quat& rotation) noexcept {
	_rotation = rotation;
	_is_transformed = false;
}

void Model::set_pos(const glm::vec3& pos) noexcept {
	_world_pos = pos;
	_is_transformed = false;
}

void Model::set_new_transform() noexcept {
	_transform = glm::mat4_cast(_rotation) *
		glm::translate(glm::mat4(1.f), _world_pos) *
		glm::scale(glm::mat4(1.f), _size);
	_is_copied.assign(_is_copied.size(), false);
}

Model::Model(const Mesh* mesh,
	uint32_t instance_index,
	uint32_t first_vertex,
	uint32_t first_index,
	std::vector<void*> _buffer_ptr) noexcept :
	WorldObject(mesh->get_pos(), mesh->get_size(), mesh->get_rotation()),
	_vertices_count(mesh->get_vertices_count()),
	_indices_count(mesh->get_indices_count()),
	_first_buffer_vertex(first_vertex),
	_first_buffer_index(first_index),
	_instance_index(instance_index),
	_model_transform_ptr(_buffer_ptr){

	set_new_transform();
	for (void* ptr : _buffer_ptr) {
		memcpy(ptr, &_transform, sizeof(glm::mat4));
	}
}

std::vector<VkDescriptorSetLayoutBinding> Model::get_bindings() noexcept {
	std::vector<VkDescriptorSetLayoutBinding> bindings(1);

	bindings[0].binding = 1;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	return bindings;
}