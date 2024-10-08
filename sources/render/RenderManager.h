#pragma once
#include "RenderUnitBase.h"

class CurrentFrameData {
public:
	VkDescriptorSet global_UBO;
};

struct RenderManagerCreateInfo {
	const std::shared_ptr<class Scene>& scene;
	const class CameraBase& camera;
	const class Window& window;
	struct GuiInfo& gui_info;
	const std::shared_ptr<class MaterialManager>& material_manager;
};

class RenderManager {
private:
	const class CameraBase& _camera;
	const std::shared_ptr<class Scene> _scene;
	const std::shared_ptr<class MaterialManager> _material_manager;
	std::vector<RenderUnitBase*> _render_units;

	std::vector<VulkanBuffer*> _global_uniform_buffers;
	std::vector<char*> _global_uniform_memory_ptrs;

	std::vector<VkDescriptorSet> _descriptor_sets;
	VkDescriptorPool _descriptor_pool;
	VkDescriptorSetLayout _descriptor_set_layout;

private:

	struct CreateImagesInfo {
		std::shared_ptr<VulkanImage> depth_image;
		std::shared_ptr<VulkanImageView> depth_image_view;
		std::shared_ptr<VulkanImage> hdr_image;
		std::shared_ptr<VulkanImageView> hdr_image_view;
		std::shared_ptr<VulkanTexture2D> bright_image;
		std::shared_ptr<VulkanTexture2D> staging_color_image;
		std::shared_ptr<VulkanTexture2D> equirectangular_env_map;
		std::shared_ptr<VulkanCube> skybox;
	};

	void create_images(CreateImagesInfo& create_images_info);
	void create_buffers();
	void create_descritor_tools();
public:
	RenderManager(const RenderManagerCreateInfo& render_manager_create_info) noexcept;

	void update_descriptor_sets(VkCommandBuffer command_buffer);
	void render(VkCommandBuffer command_buffer);
	//global UBO
	static std::vector<VkDescriptorSetLayoutBinding> get_bindings() noexcept;

	~RenderManager();
};