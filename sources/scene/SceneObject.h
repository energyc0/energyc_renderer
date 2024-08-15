#pragma once

#include "VulkanDataObjects.h"
#include <glm/glm.hpp>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 normal;

	static std::vector<VkVertexInputAttributeDescription> get_attribute_description();
	static std::vector<VkVertexInputBindingDescription> get_binding_description();
};

class SceneObject {
protected:
	SceneObject() noexcept {}
public:
	virtual ~SceneObject() {}

};

class RenderObject : public SceneObject {
protected:
	glm::vec3 _world_pos;

public:
	RenderObject() noexcept;
	RenderObject(const glm::vec3& pos) noexcept;

	inline glm::vec3 get_pos() { return _world_pos; }
	virtual void set_pos(const glm::vec3& pos) noexcept;

	virtual void draw(VkCommandBuffer command_buffer) = 0;
};
//
//struct Mesh {
//	std::vector<Vertex> vertices;
//	std::vector<uint32_t> indices;
//};

class Model : public RenderObject {
protected:
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;

protected:
	static Model load_mesh(const char* filename) noexcept;

public:
	Model(const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices) noexcept;
	Model(const char* filename) noexcept;

	inline uint32_t get_vertices_count() const noexcept { return _vertices.size(); }
	inline uint32_t get_indices_count() const noexcept { return _indices.size(); }
	inline const Vertex* get_vertex_data()const noexcept { return _vertices.data(); }
	inline const uint32_t* get_index_data()const noexcept { return _indices.data(); }

	virtual void draw(VkCommandBuffer command_buffer) noexcept;
	virtual void draw_indexed(VkCommandBuffer command_buffer, int32_t vertex_offset) noexcept;
};