#pragma once

#include "RenderUnitBase.h"

struct RenderUnitSolidCreateInfo {
	const class Scene& scene;
	const class CameraBase& camera;
	const class Window& window;
	struct GuiInfo& gui_info;
	const std::shared_ptr<VulkanImage>& depth_image;
	const std::shared_ptr<VulkanImageView>& depth_image_view;
	VkDescriptorSetLayout global_UBO_descriptor_set_layout;
};

class RenderUnitSolid : public RenderUnitBase{
private:
	const class CameraBase& _camera;

	class RendererSolid* _renderer_solid;
	class RendererGui* _renderer_gui;

	VulkanMultipleImageViews* _image_views;
	std::shared_ptr<VulkanImage> _depth_image;
	std::shared_ptr<VulkanImageView> _depth_image_view;

	VkPipelineLayout _pipeline_layout;

private:
	virtual void create_render_pass();
	virtual void create_framebuffers();
	void create_descriptor_tools(const RenderUnitSolidCreateInfo& unit_create_info);
	void create_images();

public:
	RenderUnitSolid(const RenderUnitSolidCreateInfo& create_info);

	virtual void fill_command_buffer(VkCommandBuffer command_buffer, const struct CurrentFrameData& frame_data);

	~RenderUnitSolid();
};