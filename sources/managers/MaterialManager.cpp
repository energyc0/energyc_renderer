#include "CommandManager.h"
#include "SyncManager.h"
#include "MaterialManager.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"

constexpr uint32_t MATERIAL_ALLOCATION_POOL = 10;
constexpr uint32_t MATERIAL_BUFFER_LIMIT = 10;

ObjectMaterial::ObjectMaterial(const std::string& name, int32_t material_index) noexcept :
	NamedObject(name), _material_index(material_index) {}

MaterialManager::Material::Material(const std::string& name,
	int32_t material_index, const VulkanBuffer& material_ubo,
	const char* albedo,
	const char* metallic,
	const char* roughness,
	const char* normal) noexcept :
	ObjectMaterial(name,material_index),
	_albedo(VK_FORMAT_R8G8B8A8_SRGB,albedo),
	_metallic(VK_FORMAT_R8G8B8A8_UNORM,metallic),
	_roughness(VK_FORMAT_R8G8B8A8_UNORM, roughness),
	_normal(VK_FORMAT_R8G8B8A8_UNORM,normal),
	_ubo_data(glm::vec3(-1.f),-1.f,-1.f,VK_TRUE),
	_material_ubo(material_ubo)	
{
	initialize_uniform_buffer();
}
MaterialManager::Material::Material(const std::string& name,
	int32_t material_index, const VulkanBuffer& material_ubo,
	glm::vec3 albedo,
	float metallic,
	float roughness) : ObjectMaterial(name,material_index),
	_ubo_data(albedo,metallic,roughness, VK_FALSE),
	_material_ubo(material_ubo) 
{
	initialize_uniform_buffer();
}

MaterialManager::Material::Material(Material&& material) noexcept :
	ObjectMaterial(material._name, material._material_index),
	_albedo(std::move(material._albedo)),
	_roughness(std::move(material._roughness)),
	_metallic(std::move(material._metallic)),
	_normal(std::move(material._normal)),
	_ubo_data(std::move(material._ubo_data)),
	_material_ubo(material._material_ubo) 
{
	initialize_uniform_buffer();
}

void MaterialManager::Material::initialize_uniform_buffer() const noexcept {
	VkDeviceSize offset = (_material_ubo.get_size() / MATERIAL_BUFFER_LIMIT) * _material_index;
	auto data = get_uniform_data();

	auto cmd = CommandManager::begin_single_command_buffer();
	StagingBuffer::copy_buffers(cmd, &data, sizeof(MaterialUniformData), _material_ubo, 0, offset);
	CommandManager::end_single_command_buffer(cmd);
}


void MaterialManager::Material::update_uniform_buffer(VkCommandBuffer command_buffer) noexcept {
	if (_has_changed) {
		_has_changed = false;
		VkDeviceSize offset = (_material_ubo.get_size() / MATERIAL_BUFFER_LIMIT) * _material_index;
		auto data = get_uniform_data();

		StagingBuffer::copy_buffers(command_buffer, &data, sizeof(MaterialUniformData), _material_ubo, 0, offset);
	}
}

std::vector<VkDescriptorImageInfo> MaterialManager::Material::get_info() {
	std::vector<VkDescriptorImageInfo> info{
		_albedo.get_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		_metallic.get_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ,
		_roughness.get_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ,
		_normal.get_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	};
	return info;
}

MaterialManager::MaterialManager() : 
	_material_ubo(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		((sizeof(MaterialUniformData) > Core::get_min_uniform_offset_alignment()) ? 
			sizeof(MaterialUniformData) : Core::get_min_uniform_offset_alignment()) * MATERIAL_BUFFER_LIMIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {

	auto bindings = get_bindings();

	std::vector<VkDescriptorBindingFlags> binding_flags(bindings.size(), VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);

	VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_create_info{};
	binding_flags_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	binding_flags_create_info.bindingCount = binding_flags.size();
	binding_flags_create_info.pBindingFlags = binding_flags.data();

	VkDescriptorSetLayoutCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	create_info.bindingCount = bindings.size();
	create_info.pBindings = bindings.data();
	create_info.pNext = &binding_flags_create_info;
	VK_ASSERT(vkCreateDescriptorSetLayout(Core::get_device(), &create_info, nullptr, &_descriptor_set_layout), "vkCreateDescriptorSetLayout(), RendererSolid - FAILED");
	
	//create default material for all meshes
	create_new_material("Default material", glm::vec3(1.f), 0.5f, 0.5f);

	LOG_STATUS("Created MaterialManager.");
}

void MaterialManager::push_new_descriptor_pool() {
	VkDescriptorPool descriptor_pool;

	VkDescriptorPoolSize pool_sizes[2]{};
	pool_sizes[0].descriptorCount = MATERIAL_ALLOCATION_POOL * 4;
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	pool_sizes[1].descriptorCount = MATERIAL_ALLOCATION_POOL;
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.poolSizeCount = 2;
	create_info.pPoolSizes = pool_sizes;
	create_info.maxSets = MATERIAL_ALLOCATION_POOL;
	VK_ASSERT(vkCreateDescriptorPool(Core::get_device(), &create_info, nullptr, &descriptor_pool),"vkCreateDescriptorPool() - FAILED");
	_descriptor_pools.push_back(descriptor_pool);

	_descriptor_sets.resize(_descriptor_sets.size() + MATERIAL_ALLOCATION_POOL);
	std::vector< VkDescriptorSetLayout> layouts(MATERIAL_ALLOCATION_POOL, _descriptor_set_layout);
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptor_pool;
	alloc_info.descriptorSetCount = MATERIAL_ALLOCATION_POOL;
	alloc_info.pSetLayouts = layouts.data();
	VK_ASSERT(vkAllocateDescriptorSets(Core::get_device(),
		&alloc_info,
		_descriptor_sets.data() + _descriptor_sets.size() - MATERIAL_ALLOCATION_POOL),
		"vkAllocateDescriptorSets() - FAILED");

	_pool_allocations_left = MATERIAL_ALLOCATION_POOL;
	LOG_STATUS("MaterialManager - created new descriptor pool.");
}

std::vector<VkDescriptorSetLayoutBinding> MaterialManager::get_bindings() {
	std::vector<VkDescriptorSetLayoutBinding> bindings(2);

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 4;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	
	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	return bindings;
}

void MaterialManager::Material::show_gui_info() noexcept {
	ImGui::BeginChild(_name.c_str(), ImVec2(0, 0),
		ImGuiChildFlags_AutoResizeX |
		ImGuiChildFlags_AutoResizeY |
		ImGuiChildFlags_AlwaysAutoResize |
		ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_FrameStyle);

	ImGui::Text(_name.c_str());
	if (_ubo_data.albedo != glm::vec3(-1.f)) {
		_has_changed = ImGui::SliderFloat3("Albedo", glm::value_ptr(_ubo_data.albedo), 0.0f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp) || _has_changed;
	}
	if (_ubo_data.metallic != -1.f) {
		_has_changed = ImGui::SliderFloat("Metallic", &_ubo_data.metallic, 0.0f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp) || _has_changed;
	}
	if (_ubo_data.roughness != -1.f) {
		_has_changed = ImGui::SliderFloat("Roughness", &_ubo_data.roughness, 0.0f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp) || _has_changed;
	}

	ImGui::EndChild();
}

void MaterialManager::update_uniform_buffer(VkCommandBuffer command_buffer) noexcept {
	for (auto& material : _materials) {
		material->update_uniform_buffer(command_buffer);
	}
}

void MaterialManager::show_materials_gui_info() const noexcept {
	for (auto& material : _materials) {
		material->show_gui_info();
	}
}

ObjectMaterial MaterialManager::create_new_material(const std::string& name,
	const char* albedo,
	const char* metallic,
	const char* roughness,
	const char* normal) {
	if (_pool_allocations_left == 0) {
		push_new_descriptor_pool();
	}

	int32_t material_index = _descriptor_sets.size() - _pool_allocations_left;

	Material* material = new Material(name, material_index,_material_ubo, albedo, metallic, roughness, normal);
	_materials.emplace_back(material);

	auto image_info = material->get_info();
	VkDeviceSize offset = (_material_ubo.get_size() / MATERIAL_BUFFER_LIMIT) * material_index;
	auto buffer_info = _material_ubo.get_info(offset, sizeof(MaterialUniformData));

	VkWriteDescriptorSet write[2]{};
	write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[0].descriptorCount = image_info.size();
	write[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write[0].dstBinding = 0;
	write[0].dstArrayElement = 0;
	write[0].dstSet = _descriptor_sets[material_index];
	write[0].pImageInfo = image_info.data();

	write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[1].descriptorCount = 1;
	write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write[1].dstBinding = 1;
	write[1].dstArrayElement = 0;
	write[1].dstSet = _descriptor_sets[material_index];
	write[1].pBufferInfo = &buffer_info;

	vkUpdateDescriptorSets(Core::get_device(), 2, write, 0, 0);

	_pool_allocations_left--;
	LOG_STATUS("Created new material: ", name);
	return material->get_object_material();
}

ObjectMaterial MaterialManager::create_new_material(const std::string& name,
	const glm::vec3& albedo,
	float metallic,
	float roughness) {
	if (_pool_allocations_left == 0) {
		push_new_descriptor_pool();
	}

	int32_t material_index = _descriptor_sets.size() - _pool_allocations_left;

	Material* material = new Material(name, material_index,_material_ubo, albedo, metallic, roughness);
	_materials.emplace_back(material);
	VkDeviceSize offset = (_material_ubo.get_size() / MATERIAL_BUFFER_LIMIT) * material_index;
	auto buffer_info = _material_ubo.get_info(offset, sizeof(MaterialUniformData));


	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write.dstBinding = 1;
	write.dstArrayElement = 0;
	write.dstSet = _descriptor_sets[material_index];
	write.pBufferInfo = &buffer_info;

	vkUpdateDescriptorSets(Core::get_device(), 1, &write, 0, 0);

	_pool_allocations_left--;
	LOG_STATUS("Created new material: ", name);
	return material->get_object_material();
}

MaterialManager::~MaterialManager() {
	for (auto& material : _materials) {
		delete material;
	}
	vkDestroyDescriptorSetLayout(Core::get_device(), _descriptor_set_layout, nullptr);
	for (auto pool : _descriptor_pools) {
		vkDestroyDescriptorPool(Core::get_device(), pool, nullptr);
	}
}