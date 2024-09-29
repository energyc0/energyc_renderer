#pragma once
#include "RendererBase.h"
#include "VulkanDataObjects.h"

struct RendererEquirectangularProjCreateInfo {
	VkRenderPass render_pass;
	const std::shared_ptr<VulkanTexture2D>& equirectangular_env_map;
};

class RendererEquirectangularProj : public RendererBaseExt {
private:
	void create_descriptor_tools(const RendererEquirectangularProjCreateInfo& renderer_create_info);
	void create_graphics_pipeline(const RendererEquirectangularProjCreateInfo& renderer_create_info);

public:
	RendererEquirectangularProj(const RendererEquirectangularProjCreateInfo& renderer_create_info);

	virtual void fill_command_buffer(VkCommandBuffer command_buffer);

	~RendererEquirectangularProj();
};