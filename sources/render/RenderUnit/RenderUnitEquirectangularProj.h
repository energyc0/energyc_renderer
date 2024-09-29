#pragma once

#include "RenderUnitBase.h"
#include "RendererEquirectangularProj.h"

struct RenderUnitEquirectangularProjCreateInfo {
	const std::shared_ptr<VulkanTexture2D>& equirectangular_env_map;
	const std::shared_ptr<VulkanCube>& skybox;
};

class RenderUnitEquirectangularProj : public RenderUnitBase{
private:
	std::unique_ptr<RendererEquirectangularProj> _renderer_proj;
	std::shared_ptr<VulkanTexture2D> _equirectangular_env_map;
	std::shared_ptr<VulkanCube> _skybox;

private:
	virtual void create_render_pass();
	virtual void create_framebuffers();
public:
	RenderUnitEquirectangularProj(const RenderUnitEquirectangularProjCreateInfo& create_info);

	virtual void fill_command_buffer(VkCommandBuffer command_buffer, const struct CurrentFrameData& frame_data);

	~RenderUnitEquirectangularProj();
};