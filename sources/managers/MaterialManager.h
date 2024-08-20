#pragma once

#include "VulkanDataObjects.h"
#include "SceneObject.h"

class ObjectMaterial {
protected:
	std::string _name;
	int32_t _material_index;

public:
	inline std::string get_name()const noexcept { return _name; }
	inline int32_t get_index() const noexcept { return _material_index; }

	ObjectMaterial(const std::string& name, int32_t material_index) noexcept;
};

class MaterialManager {
private:

	class Material : public ObjectMaterial {
	private:
		VulkanTexture2D _albedo;
		VulkanTexture2D _metallic;
		VulkanTexture2D _roughness;
		VulkanTexture2D _normal;
		glm::vec3 _albedo_const;
		float _metallic_const;
		float _roughness_const;
		VkBool32 _has_normal;

	public:
		Material(const std::string& name,
		int32_t material_index, 
		glm::vec3 albedo = glm::vec3(1.f),
		float metallic = 1.f,
		float roughness = 1.f);

		Material(const std::string& name,
			int32_t material_index,
			const char* albedo,
			const char* metallic,
			const char* roughness,
			const char* normal) noexcept;

		Material(Material&& material) noexcept;

		inline ObjectMaterial get_object_material() const noexcept { return ObjectMaterial(_name, _material_index); }

		std::vector<VkDescriptorImageInfo> get_info();
	};

	std::vector<Material*> _materials;
	std::vector<VkDescriptorSet> _descriptor_sets;
	std::vector<VkDescriptorPool> _descriptor_pools;
	VkDescriptorSetLayout _descriptor_set_layout;

	uint32_t _pool_allocations_left = 0;

private:
	void push_new_descriptor_pool();
public:
	MaterialManager();

	ObjectMaterial create_new_material(const std::string& name,
		const char* albedo,
		const char* metallic,
		const char* roughness,
		const char* normal);

	inline VkDescriptorSet get_material_descriptor(Model* model) const noexcept {	return _descriptor_sets[model->get_material_index()]; }
	inline VkDescriptorSetLayout get_descriptor_set_layout() const noexcept { return _descriptor_set_layout; }
	static std::vector<VkDescriptorSetLayoutBinding> get_bindings();

	~MaterialManager();
};