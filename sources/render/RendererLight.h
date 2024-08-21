#pragma once

#include "RendererBase.h"

struct RendererLightSourceCreateInfo {
	const std::shared_ptr<class Scene>& scene;
	VkDescriptorSetLayout global_UBO_descriptor_set_layout;
	VkRenderPass render_pass;
};

class RendererLightSource : public RendererBaseExt{
private:
	const std::shared_ptr<class Scene>& _scene;
private:
	void create_descriptor_tools(const RendererLightSourceCreateInfo& renderer_create_info);
	void create_graphics_pipeline(const RendererLightSourceCreateInfo& renderer_create_info);
public:
	RendererLightSource(const RendererLightSourceCreateInfo& renderer_create_info);

	virtual void fill_command_buffer(VkCommandBuffer command_buffer);
};