#pragma once
#include "RenderUnitBase.h"

class CurrentFrameData {
public:
	VkDescriptorSet global_UBO;
};

class RenderManager {
private:
	const class CameraBase& _camera;
	std::vector<RenderUnitBase*> _render_units;

	std::vector<VulkanBuffer*> _global_uniform_buffers;
	std::vector<char*> _global_uniform_memory_ptrs;

	std::vector<VkDescriptorSet> _descriptor_sets;
	VkDescriptorPool _descriptor_pool;
	VkDescriptorSetLayout _descriptor_set_layout;

private:
	void create_images(std::shared_ptr<VulkanImage>& depth_image, std::shared_ptr<VulkanImageView>& depth_image_view);
	void create_buffers();
	void create_descritor_tools();
public:
	RenderManager(class Scene& scene, const class CameraBase& camera);

	void update_descriptor_sets();
	void render(VkCommandBuffer command_buffer);

	static std::vector<VkDescriptorSetLayoutBinding> get_bindings() noexcept;

	~RenderManager();
};