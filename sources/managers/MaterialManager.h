#pragma once

#include "VulkanDataObjects.h"
#include "SceneObject.h"

class ObjectMaterial : public NamedObject {
protected:
	int32_t _material_index;

public:
	inline int32_t get_index() const noexcept { return _material_index; }

	ObjectMaterial(const std::string& name, int32_t material_index) noexcept;
};

struct MaterialUniformData {
	alignas(16) glm::vec3 albedo;
	alignas(4) float metallic;
	alignas(4) float roughness;
	alignas(4) VkBool32 has_normal;

	MaterialUniformData(const glm::vec3 albedo_, float metallic_, float roughness_, VkBool32 has_normal_) :
		albedo(albedo_), metallic(metallic_), roughness(roughness_), has_normal(has_normal_) {}
};

class MaterialManager {
private:

	class Material : public ObjectMaterial {
	private:
		VulkanTexture2D _albedo;
		VulkanTexture2D _metallic;
		VulkanTexture2D _roughness;
		VulkanTexture2D _normal;

		MaterialUniformData _ubo_data;
		bool _has_changed = false;
		const VulkanBuffer& _material_ubo;

	private:
		void initialize_uniform_buffer() const noexcept;
	public:
		Material(const std::string& name,
		int32_t material_index, const VulkanBuffer& material_ubo,
		glm::vec3 albedo = glm::vec3(1.f),
		float metallic = 1.f,
		float roughness = 1.f);

		Material(const std::string& name,
			int32_t material_index, const VulkanBuffer& material_ubo,
			const char* albedo,
			const char* metallic,
			const char* roughness,
			const char* normal) noexcept;

		Material(Material&& material) noexcept;

		inline MaterialUniformData get_uniform_data() const noexcept { return _ubo_data; }
		inline ObjectMaterial get_object_material() const noexcept { return ObjectMaterial(_name, _material_index); }

		void show_gui_info() noexcept;
		void update_uniform_buffer(VkCommandBuffer command_buffer) noexcept;

		std::vector<VkDescriptorImageInfo> get_info();
	};

	std::vector<Material*> _materials;
	std::vector<VkDescriptorSet> _descriptor_sets;
	std::vector<VkDescriptorPool> _descriptor_pools;
	VkDescriptorSetLayout _descriptor_set_layout;

	//TODO set material limit
	VulkanBuffer _material_ubo;

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

	ObjectMaterial create_new_material(const std::string& name,
		const glm::vec3& albedo,
		float metallic,
		float roughness);

	inline VkDescriptorSet get_material_descriptor(Model* model) const noexcept {	return _descriptor_sets[model->get_material_index()]; }
	inline VkDescriptorSetLayout get_descriptor_set_layout() const noexcept { return _descriptor_set_layout; }
	static std::vector<VkDescriptorSetLayoutBinding> get_bindings();
	
	void update_uniform_buffer(VkCommandBuffer command_buffer) noexcept;
	void show_materials_gui_info() const noexcept;

	~MaterialManager();
};