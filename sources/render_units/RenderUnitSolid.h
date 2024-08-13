#pragma once

#include "RenderUnitBase.h"
#include "RendererSolid.h"

class RenderUnitSolid : public RenderUnitBase{
private:
	RendererSolid* _renderer_solid;
	VulkanMultipleImageViews* _image_views;

private:
	virtual void create_render_pass();
	virtual void create_framebuffers();

public:
	RenderUnitSolid();

	virtual void fill_command_buffer(VkCommandBuffer command_buffer);

	~RenderUnitSolid();
};