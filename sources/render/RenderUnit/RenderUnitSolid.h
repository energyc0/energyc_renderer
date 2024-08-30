#pragma once

#include "RenderUnitBase.h"

struct RenderUnitSolidCreateInfo {
	const std::shared_ptr<class Scene>& scene;
	const std::shared_ptr<class MaterialManager>& material_manager;
	const class CameraBase& camera;
	const std::shared_ptr<VulkanImage>& depth_image;
	const std::shared_ptr<VulkanImageView>& depth_image_view;
	const std::shared_ptr<VulkanImage>& hdr_image;
	const std::shared_ptr<VulkanImageView>& hdr_image_view;
	const std::shared_ptr<VulkanTexture2D>& bright_image;
	VkDescriptorSetLayout global_UBO_descriptor_set_layout;
};

class RenderUnitSolid : public RenderUnitBase{
private:
	const class CameraBase& _camera;

	class RendererSolid* _renderer_solid;
	class RendererLightSource* _renderer_light;

	std::shared_ptr<VulkanImage> _depth_image;
	std::shared_ptr<VulkanImageView> _depth_image_view;
	std::shared_ptr<VulkanImage> _hdr_image;
	std::shared_ptr<VulkanImageView> _hdr_image_view;
	std::shared_ptr<VulkanTexture2D> _bright_image;

	VkPipelineLayout _pipeline_layout;

private:
	virtual void create_render_pass();
	virtual void create_framebuffers();
	void create_descriptor_tools(const RenderUnitSolidCreateInfo& unit_create_info);

public:
	RenderUnitSolid(const RenderUnitSolidCreateInfo& create_info);

	virtual void fill_command_buffer(VkCommandBuffer command_buffer, const struct CurrentFrameData& frame_data);

	~RenderUnitSolid();
};