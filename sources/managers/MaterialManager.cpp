#include "MaterialManager.h"

constexpr uint32_t MATERIAL_ALLOCATION_POOL = 10;

ObjectMaterial::ObjectMaterial(const std::string& name, int32_t material_index) noexcept :
	_name(name), _material_index(material_index) {}

MaterialManager::Material::Material(const std::string& name,
	int32_t material_index,
	const char* albedo,
	const char* metallic,
	const char* roughness,
	const char* normal) noexcept :
	ObjectMaterial(name,material_index),
	_albedo(VK_FORMAT_R8G8B8A8_SRGB,albedo),
	_metallic(VK_FORMAT_R8G8B8A8_UNORM,metallic),
	_roughness(VK_FORMAT_R8G8B8A8_UNORM, roughness),
	_normal(VK_FORMAT_R8G8B8A8_UNORM,normal),
	_albedo_const(glm::vec3(1.f)),
	_metallic_const(1.f),
	_roughness_const(1.f),
	_has_normal(VK_TRUE){}

MaterialManager::Material::Material(const std::string& name,
	int32_t material_index, 
	glm::vec3 albedo,
	float metallic,
	float roughness) : ObjectMaterial(name,material_index),
	_albedo_const(albedo),
	_metallic_const(metallic),
	_roughness_const(roughness),
	_has_normal(VK_FALSE) {}

MaterialManager::Material::Material(Material&& material) noexcept :
	ObjectMaterial(material._name, material._material_index),
	_albedo(std::move(material._albedo)),
	_roughness(std::move(material._roughness)),
	_metallic(std::move(material._metallic)),
	_normal(std::move(material._normal)) {}

std::vector<VkDescriptorImageInfo> MaterialManager::Material::get_info() {
	std::vector<VkDescriptorImageInfo> info{
		_albedo.get_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		_metallic.get_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ,
		_roughness.get_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ,
		_normal.get_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	};
	return info;
}

MaterialManager::MaterialManager() {
	auto bindings = get_bindings();
	VkDescriptorSetLayoutCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	create_info.bindingCount = bindings.size();
	create_info.pBindings = bindings.data();
	VK_ASSERT(vkCreateDescriptorSetLayout(Core::get_device(), &create_info, nullptr, &_descriptor_set_layout), "vkCreateDescriptorSetLayout(), RendererSolid - FAILED");

	LOG_STATUS("Created MaterialManager.");
}

void MaterialManager::push_new_descriptor_pool() {
	VkDescriptorPool descriptor_pool;

	VkDescriptorPoolSize pool_size{};
	pool_size.descriptorCount = MATERIAL_ALLOCATION_POOL * 4;
	pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	VkDescriptorPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.poolSizeCount = 1;
	create_info.pPoolSizes = &pool_size;
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
	std::vector<VkDescriptorSetLayoutBinding> bindings(1);

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 4;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	return bindings;
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

	Material* material = new Material(name, material_index, albedo, metallic, roughness, normal);
	_materials.emplace_back(material);

	auto image_info = material->get_info();


	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorCount = image_info.size();
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.dstBinding = 0;
	write.dstArrayElement = 0;
	write.dstSet = _descriptor_sets[material_index];
	write.pImageInfo = image_info.data();

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