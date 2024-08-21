#pragma once

#include "VulkanDataObjects.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Vertex {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 color;
	alignas(16) glm::vec3 normal;
	alignas(16) glm::vec2 uv;
	alignas(16) glm::vec3 tangent;

	static std::vector<VkVertexInputAttributeDescription> get_attribute_description();
	static std::vector<VkVertexInputBindingDescription> get_binding_description();
};

class NamedObject {
protected:
	std::string _name;
	NamedObject(const std::string& name) noexcept : _name(name) {}
	NamedObject(std::string&& name) noexcept : _name(std::move(name)) {}

public:
	inline std::string get_name() const noexcept { return _name; }
};

class SceneObject : public NamedObject{
protected:
	SceneObject(const std::string& name) noexcept : NamedObject(name){}
	SceneObject(std::string&& name) noexcept : NamedObject(std::move(name)) {}

public:
	virtual ~SceneObject() {}

	virtual void display_gui_info() const noexcept = 0;
};

class PositionedObject {
protected:
	glm::vec3 _world_pos;

protected:
	PositionedObject() noexcept;
	PositionedObject(const glm::vec3& pos) noexcept;

	virtual ~PositionedObject() {}
public:
	inline glm::vec3 get_pos() const noexcept { return _world_pos; }
	virtual void set_pos(const glm::vec3& pos) noexcept;
};

class WorldObject : public PositionedObject {
protected:
	glm::vec3 _size;
	glm::quat _rotation;

public:
	WorldObject(glm::vec3 world_pos = glm::vec3(0.f),
		glm::vec3 size = glm::vec3(1.f),
		glm::quat rotation = glm::vec3(0.f));

	virtual void set_size(const glm::vec3& size) noexcept;
	virtual void set_rotation(const glm::quat& rotation) noexcept;

	inline glm::vec3 get_size()const noexcept { return _size; }
	inline glm::quat get_rotation()const noexcept { return _rotation; }
};

class Mesh : public WorldObject, public NamedObject {
private:
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;

	int32_t _material_index;

	static Mesh load_mesh(const std::string& filename) noexcept;
	Mesh(std::string&& name) noexcept;
public:
	Mesh(const Mesh& mesh) noexcept;
	Mesh(Mesh&& mesh) noexcept;

	Mesh(const std::string& name,const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
		glm::vec3 world_pos = glm::vec3(0.f),
		glm::vec3 size = glm::vec3(1.f),
		glm::quat rotation = glm::vec3(0.f),
		int32_t material_index = -1) noexcept;
	Mesh(std::string&& name, std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices,
		glm::vec3 world_pos = glm::vec3(0.f),
		glm::vec3 size = glm::vec3(1.f),
		glm::quat rotation = glm::vec3(0.f),
		int32_t material_index = -1) noexcept;
	Mesh(const char* filename,
		glm::vec3 world_pos = glm::vec3(0.f),
		glm::vec3 size = glm::vec3(1.f),
		glm::quat rotation = glm::vec3(0.f)) noexcept;

	void set_material(const class ObjectMaterial& material) noexcept;
	inline int32_t get_material_index() const noexcept { return _material_index; }
	inline uint32_t get_vertices_count() const noexcept { return _vertices.size(); }
	inline uint32_t get_indices_count() const noexcept { return _indices.size(); }
	inline const Vertex* get_vertex_data() const noexcept { return _vertices.data(); }
	inline const uint32_t* get_index_data() const noexcept { return _indices.data(); }
};

class Model : public WorldObject, public SceneObject {
protected:
	glm::mat4 _transform;

	uint32_t _vertices_count;
	uint32_t _indices_count;
	uint32_t _instance_index;
	uint32_t _first_buffer_vertex;
	uint32_t _first_buffer_index;

	std::vector<void*> _model_transform_ptr;
	std::vector<bool> _is_copied = std::vector<bool>(Core::get_swapchain_image_count(),true);
	bool _is_transformed = true;

	int32_t _material_index;
protected:

	void set_new_transform() noexcept;
public:
	Model(const Mesh* mesh,
		uint32_t instance_index,
		uint32_t first_vertex,
		uint32_t first_index,
		std::vector<void*> _buffer_ptr) noexcept;

	static std::vector<VkDescriptorSetLayoutBinding> get_bindings() noexcept;
	
	virtual void set_pos(const glm::vec3& pos) noexcept;
	virtual void set_size(const glm::vec3& size) noexcept;
	virtual void set_rotation(const glm::quat& rotation) noexcept;
	virtual void set_material(const class ObjectMaterial& material) noexcept;

	inline int32_t get_material_index() const noexcept { return _material_index; }

	inline void copy_to_buffer() noexcept { memcpy(_model_transform_ptr[Core::get_current_frame()], &_transform, sizeof(glm::mat4)); }

	virtual inline void draw(VkCommandBuffer command_buffer) noexcept {
		if (!_is_copied[Core::get_current_frame()]) {
			if (!_is_transformed) {
				set_new_transform();
			}
			copy_to_buffer();
		}
		vkCmdDraw(command_buffer, _vertices_count, 1, _first_buffer_vertex, _instance_index);
	}
	virtual inline void draw_indexed(VkCommandBuffer command_buffer) noexcept {
		if (!_is_copied[Core::get_current_frame()]) {
			if (!_is_transformed) {
				set_new_transform();
			}
			copy_to_buffer();
		}
		vkCmdDrawIndexed(command_buffer, _indices_count, 1, _first_buffer_index, 0, _instance_index);
	}

	virtual void display_gui_info() const noexcept;
};

struct PointLightData {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 color;
};

class PointLight : public PositionedObject, public SceneObject {
protected:
	glm::vec3 _color;
public:
	PointLight(const std::string& name, const glm::vec3& position = glm::vec3(0.f), const glm::vec3& color = glm::vec3(1.f));

	inline PointLightData get_data() const noexcept { return PointLightData{ _world_pos,_color }; }

	static std::vector<VkDescriptorSetLayoutBinding> get_bindings() noexcept;

	virtual void display_gui_info() const noexcept;
};