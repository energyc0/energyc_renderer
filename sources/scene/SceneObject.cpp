#include "SceneObject.h"

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

RenderObject::RenderObject() noexcept : _world_pos(0.f){}
RenderObject::RenderObject(const glm::vec3& pos) noexcept  : _world_pos(pos) {}

void RenderObject::set_pos(const glm::vec3& pos) noexcept {
	_world_pos = pos;
}

Model Model::load_mesh(const char* filename) noexcept {
	return Model({}, {});
}

Model::Model(const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices) noexcept : _vertices(vertices), _indices(indices) {}

Model::Model(const char* filename) noexcept : Model(load_mesh(filename)) {}

void Model::draw(VkCommandBuffer command_buffer) noexcept  {
	vkCmdDraw(command_buffer, _vertices.size(), 1, 0, 0);
}

void Model::draw_indexed(VkCommandBuffer command_buffer, int32_t vertex_offset) noexcept {
	vkCmdDrawIndexed(command_buffer, _indices.size(), 1, 0, vertex_offset, 0);
}