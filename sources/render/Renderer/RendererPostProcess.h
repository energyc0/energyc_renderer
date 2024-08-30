#pragma once
#include "RendererBase.h"

struct RendererPostProcessCreateInfo {
	VkRenderPass render_pass;
	const std::shared_ptr<class VulkanImageView>& hdr_color_image_view;
	const std::shared_ptr<class VulkanTexture2D>& staging_color_image;
};

class RendererPostProcess : public RendererBaseExt {
	//hdr color input attachment
	VkDescriptorSetLayout _descriptor_set_layout;
	VkDescriptorSet _descriptor_set;
private:
	void create_descriptor_tools(const RendererPostProcessCreateInfo& renderer_create_info);
	void create_graphics_pipeline(const RendererPostProcessCreateInfo& renderer_create_info) noexcept;

public:
	RendererPostProcess(const RendererPostProcessCreateInfo& renderer_create_info);

	virtual void fill_command_buffer(VkCommandBuffer command_buffer);

	static std::vector<VkDescriptorSetLayoutBinding> get_bindings() noexcept;

	~RendererPostProcess();
};