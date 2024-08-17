#pragma once

#include "VulkanDataObjects.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Vertex {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 color;
	alignas(16) glm::vec3 normal;

	static std::vector<VkVertexInputAttributeDescription> get_attribute_description();
	static std::vector<VkVertexInputBindingDescription> get_binding_description();
};

class SceneObject {
protected:
	SceneObject() noexcept {}
public:
	virtual ~SceneObject() {}

};

class WorldObject : public SceneObject {
protected:
	glm::vec3 _world_pos;

protected:
	WorldObject() noexcept;
	WorldObject(const glm::vec3& pos) noexcept;

public:
	inline glm::vec3 get_pos() { return _world_pos; }
	virtual void set_pos(const glm::vec3& pos) noexcept;
};

class Mesh : public SceneObject{
private:
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;

	static Mesh load_mesh(const char* filename) noexcept;
public:
	Mesh() noexcept;
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) noexcept;
	Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices) noexcept;
	Mesh(const char* filename) noexcept;

	inline uint32_t get_vertices_count() const noexcept { return _vertices.size(); }
	inline uint32_t get_indices_count() const noexcept { return _indices.size(); }
	inline const Vertex* get_vertex_data()const noexcept { return _vertices.data(); }
	inline const uint32_t* get_index_data()const noexcept { return _indices.data(); }
};

class Model : public WorldObject {
protected:
	glm::vec3 _size = glm::vec3(1.f);
	glm::quat _rotation = glm::vec3(0.f);

	uint32_t _vertices_count;
	uint32_t _indices_count;
	uint32_t _instance_index;
	uint32_t _first_buffer_vertex;
	uint32_t _first_buffer_index;

public:
	Model(const Mesh& mesh,
		uint32_t instance_index,
		uint32_t first_vertex,
		uint32_t first_index,
		glm::vec3 world_pos = glm::vec3(0.f)) noexcept;

	static std::vector<VkDescriptorSetLayoutBinding> get_bindings() noexcept;

	virtual inline void draw(VkCommandBuffer command_buffer) noexcept {
		vkCmdDraw(command_buffer, _vertices_count, 1, _first_buffer_vertex, 0);
	}
	virtual inline void draw_indexed(VkCommandBuffer command_buffer) noexcept {
		vkCmdDrawIndexed(command_buffer, _indices_count, 1, _first_buffer_index, 0, 0);
	}
};