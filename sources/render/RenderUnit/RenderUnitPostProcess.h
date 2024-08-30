#pragma once
#include "RenderUnitBase.h"

struct RenderUnitPostProcessCreateInfo {
	const class Window& window;
	struct GuiInfo& gui_info;

	const std::shared_ptr<VulkanImage>& hdr_color_image;
	const std::shared_ptr<VulkanImageView>& hdr_color_image_view;
	// for blur output
	const std::shared_ptr<VulkanTexture2D>& staging_color_image;
	const std::shared_ptr<VulkanTexture2D>& _bright_color_image;
};

class RenderUnitPostProcess : public RenderUnitBase {
private:
	std::shared_ptr<VulkanImage> _hdr_color_image;
	std::shared_ptr<VulkanImageView> _hdr_color_image_view;
	std::shared_ptr<VulkanTexture2D> _bright_color_image;
	// for blur output
	std::shared_ptr<VulkanTexture2D> _staging_color_image;

	VulkanMultipleImageViews* _image_views;

	std::unique_ptr<class RendererBlur> _renderer_blur;
	std::unique_ptr<class RendererPostProcess> _renderer_post_process;
	std::unique_ptr<class RendererGui> _renderer_gui;
private:
	void create_images();

	virtual void create_render_pass();
	virtual void create_framebuffers();

public:
	virtual void fill_command_buffer(VkCommandBuffer command_buffer, const struct CurrentFrameData& frame_data);


	RenderUnitPostProcess(const RenderUnitPostProcessCreateInfo& create_info);
	~RenderUnitPostProcess();
};