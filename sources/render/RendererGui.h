#pragma once

#include "RendererBase.h"

struct GuiInfo {
	float delta_time;
};

class RendererGui : public RendererBase{
private:
	GuiInfo& _gui_info;
private:
	void create_descriptor_tools();
public:
	RendererGui(const class Window& window, VkRenderPass render_pass, GuiInfo& info);

	virtual void fill_command_buffer(VkCommandBuffer command_buffer);

	~RendererGui();
};