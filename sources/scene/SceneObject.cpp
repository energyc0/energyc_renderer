#include "SceneObject.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "MaterialManager.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

std::unordered_map<std::string, uint32_t> NamedObject::_model_names;

std::vector<VkVertexInputAttributeDescription> Vertex::get_attribute_description() {
	std::vector<VkVertexInputAttributeDescription> descriptions(5);
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

	descriptions[3].binding = 0;
	descriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
	descriptions[3].location = 3;
	descriptions[3].offset = offsetof(Vertex, uv);

	descriptions[4].binding = 0;
	descriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	descriptions[4].location = 4;
	descriptions[4].offset = offsetof(Vertex, tangent);

	return descriptions;
}

std::vector<VkVertexInputBindingDescription> Vertex::get_binding_description() {
	std::vector<VkVertexInputBindingDescription> bindings(1);

	bindings[0].binding = 0;
	bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	bindings[0].stride = sizeof(Vertex);

	return bindings;
}

NamedObject::NamedObject(const std::string& name) noexcept : _name(name) {
	uint32_t k = _model_names[_name]++;
	if (k > 1) {
		_name += "." + std::to_string(k);
	}
}

NamedObject::NamedObject(std::string&& name) noexcept : _name(std::move(name)) {
	uint32_t k = _model_names[_name]++;
	if (k > 1) {
		_name += "." + std::to_string(k);
	}
}

PositionedObject::PositionedObject() noexcept : _world_pos(0.f){}
PositionedObject::PositionedObject(const glm::vec3& pos) noexcept  : _world_pos(pos) {}

void PositionedObject::set_pos(const glm::vec3& pos) noexcept {
	_world_pos = pos;
	_is_transformed = true;
}

Mesh Mesh::load_mesh(const std::string& filename) noexcept {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	std::string mtl_dir = filename.substr(0,filename.find_last_of('/'));

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(),mtl_dir.c_str())) {
		LOG_ERROR(warn, '\n', err);
	}
	if (!warn.empty()) {
		LOG_WARNING(warn);
	}
	Mesh mesh(std::move(shapes[0].name));
	uint32_t current_index = 0;
	for (const auto& shape : shapes) {
		for (auto& face_vert_k : shape.mesh.num_face_vertices) {
			if (face_vert_k != 3) {
				LOG_ERROR("Failed to load the mesh. Its face is not a triangle.");
			}
			for (uint32_t v = 0; v < face_vert_k; v++) {
				auto index = shape.mesh.indices[current_index + v];
				Vertex vertex{};

				vertex.pos.x = attrib.vertices[3 * index.vertex_index + 0];
				vertex.pos.y = attrib.vertices[3 * index.vertex_index + 1];
				vertex.pos.z = attrib.vertices[3 * index.vertex_index + 2];

				if (!attrib.colors.empty()) {
					vertex.color.x = attrib.colors[3 * index.vertex_index + 0];
					vertex.color.y = attrib.colors[3 * index.vertex_index + 1];
					vertex.color.z = attrib.colors[3 * index.vertex_index + 2];
				}
				else {
					vertex.color = glm::vec3(1.0f);
				}

				if (!attrib.normals.empty()) {
					vertex.normal.x = attrib.normals[3 * index.normal_index + 0];
					vertex.normal.y = attrib.normals[3 * index.normal_index + 1];
					vertex.normal.z = attrib.normals[3 * index.normal_index + 2];
					vertex.normal = glm::normalize(vertex.normal);
				}

				if (!attrib.texcoords.empty()) {
					//vulkan -y coordinate
					vertex.uv.x = attrib.texcoords[2 * index.texcoord_index + 0];
					vertex.uv.y = 1.0 - attrib.texcoords[2 * index.texcoord_index + 1];
				}
				mesh._vertices.emplace_back(std::move(vertex));
				mesh._indices.push_back(current_index + v);
			}
			//generate tangent

			Vertex& p1 = mesh._vertices[current_index];
			Vertex& p2 = mesh._vertices[current_index+1];
			Vertex& p3 = mesh._vertices[current_index+2];

			glm::vec2 d_uv1 = p1.uv - p2.uv;
			glm::vec2 d_uv2 = p3.uv - p2.uv;
			glm::vec3 edge1 = p1.pos - p2.pos;
			glm::vec3 edge2 = p3.pos - p2.pos;

			glm::mat2x2 uv_mat_inverse = glm::inverse(glm::mat2x2(d_uv1.x, d_uv2.x, d_uv1.y, d_uv2.y));
			glm::mat3x2 edge_mat = glm::mat3x2(edge1.x, edge2.x, edge1.y, edge2.y, edge1.z, edge2.z);
			glm::mat3x2 TB_mat = uv_mat_inverse* edge_mat;

			p3.tangent = p2.tangent = p1.tangent = glm::vec3(TB_mat[0].x, TB_mat[1].x, TB_mat[1].x);
			current_index += 3;
		}
	}

	return mesh;
}

WorldObject::WorldObject(glm::vec3 world_pos,glm::vec3 size,glm::quat rotation):
	PositionedObject(world_pos), _size(size), _rotation(rotation){}

void WorldObject::set_size(const glm::vec3& size) noexcept {
	_size = size;
	_is_transformed = true;
}

void WorldObject::set_rotation(const glm::quat& rotation) noexcept {
	_rotation = rotation;
	_is_transformed = true;
}

Mesh::Mesh(std::string&& name) noexcept :
	NamedObject(std::move(name)), _vertices(), _indices(), _material_index(-1) {}

Mesh::Mesh(const Mesh& mesh) noexcept :
	NamedObject(mesh._name),
	_vertices(mesh._vertices),
	_indices(mesh._indices),
	_material_index(mesh._material_index){}

Mesh::Mesh(Mesh&& mesh) noexcept :
	NamedObject(std::move(mesh._name)),
	_vertices(std::move(mesh._vertices)),
	_indices(std::move(mesh._indices)),
	_material_index(std::move(mesh._material_index)) {}

Mesh::Mesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
	glm::vec3 world_pos,glm::vec3 size,	glm::quat rotation,
	int32_t material_index) noexcept :
	NamedObject(name),
	WorldObject(world_pos,size,rotation),
	_vertices(vertices), _indices(indices), _material_index(material_index) {}

Mesh::Mesh(std::string&& name, std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices,
	glm::vec3 world_pos,
	glm::vec3 size,
	glm::quat rotation,
	int32_t material_index) noexcept :
	NamedObject(name),
	WorldObject(world_pos, size, rotation),
	_vertices(std::move(vertices)), _indices(std::move(indices)), _material_index(material_index) {}

Mesh::Mesh(const char* filename,
	glm::vec3 world_pos,
	glm::vec3 size,
	glm::quat rotation) noexcept :
	Mesh(load_mesh(filename)) {
	_world_pos = world_pos;
	_size = size;
	_rotation = rotation;
}

void Mesh::set_material(const ObjectMaterial & material) noexcept { _material_index = material.get_index(); }

void Model::set_new_transform() noexcept {
	_transform = glm::mat4_cast(_rotation) *
		glm::translate(glm::mat4(1.f), _world_pos) *
		glm::scale(glm::mat4(1.f), _size);
	_is_copied.assign(_is_copied.size(), false);
}

void Model::set_material(const ObjectMaterial& material) noexcept {
	_material_index = material.get_index();
}

Model::Model(const Mesh* mesh,
	uint32_t instance_index,
	uint32_t first_vertex,
	uint32_t first_index,
	std::vector<void*> _buffer_ptr) noexcept :
	SceneObject(mesh->get_name()),
	WorldObject(mesh->get_pos(), mesh->get_size(), mesh->get_rotation()),
	_vertices_count(mesh->get_vertices_count()),
	_indices_count(mesh->get_indices_count()),
	_first_buffer_vertex(first_vertex),
	_first_buffer_index(first_index),
	_instance_index(instance_index),
	_model_transform_ptr(_buffer_ptr),
	_material_index(mesh->get_material_index()){

	set_new_transform();
	for (void* ptr : _buffer_ptr) {
		memcpy(ptr, &_transform, sizeof(glm::mat4));
	}
}

void Model::display_gui_info() noexcept {
	ImGui::BeginChild(_name.c_str(),ImVec2(0.f,0.f),
		ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize| ImGuiChildFlags_AlwaysUseWindowPadding);
	ImGui::Text(_name.c_str());
	if (ImGui::SliderFloat3("Position: ", glm::value_ptr(_world_pos), -10.f, 10.f, "%.6f", ImGuiSliderFlags_AlwaysClamp) ||
		ImGui::SliderFloat3("Rotation: ", glm::value_ptr(_rotation), -1.f, 1.f, "%.6f", ImGuiSliderFlags_AlwaysClamp) ||
		ImGui::SliderFloat3("Size: ", glm::value_ptr(_size), 0.f, 10.f, "%.6f", ImGuiSliderFlags_AlwaysClamp)) {
		_is_transformed = true;
	}
	ImGui::EndChild();
}

std::vector<VkDescriptorSetLayoutBinding> Model::get_bindings() noexcept {
	std::vector<VkDescriptorSetLayoutBinding> bindings(1);

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	return bindings;
}

PointLight::PointLight(const std::string& name, const glm::vec3& position, const glm::vec3& color, float radius) :
	SceneObject(name), PositionedObject(position), _color(color), _radius(radius), _is_copied(Core::get_swapchain_image_count(), false) {}

std::vector<VkDescriptorSetLayoutBinding> PointLight::get_bindings() noexcept {
	std::vector<VkDescriptorSetLayoutBinding> bindings(1);

	bindings[0].binding = 1;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;

	return bindings;
}


void PointLight::display_gui_info() noexcept {
	ImGui::BeginChild(_name.c_str(), ImVec2(0.f, 0.f), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize);
	ImGui::Text(_name.c_str());
	if (ImGui::SliderFloat3("Position: ", glm::value_ptr(_world_pos), -10.f, 10.f, "%.6f", ImGuiSliderFlags_AlwaysClamp) ||
		ImGui::SliderFloat("Radius: ", &_radius, 0.f, 5.f, "%.6f", ImGuiSliderFlags_AlwaysClamp)) {
		_is_copied.assign(_is_copied.size(), false);
	}
	ImGui::EndChild();
}