#pragma once
#include "RendererBase.h"

struct RendererBlurCreateInfo {
	VkRenderPass render_pass;
	const std::shared_ptr<class VulkanTexture2D>& bright_image;
};

class RendererBlur : public RendererBaseExt {
private:
	//bright color combined image sampler
	VkDescriptorSetLayout _descriptor_set_layout;
	VkDescriptorSet _descriptor_set;
private:
	void create_descriptor_tools(const RendererBlurCreateInfo& renderer_create_info) noexcept;
	void create_graphics_pipeline(const RendererBlurCreateInfo& renderer_create_info) noexcept;
public:
	RendererBlur(const RendererBlurCreateInfo& create_info);

	virtual void fill_command_buffer(VkCommandBuffer command_buffer);

	static VkDescriptorSetLayoutBinding get_bindings() noexcept;

	~RendererBlur();
};