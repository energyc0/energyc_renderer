#pragma once

#include "RendererBase.h"
#include <memory>

struct GuiInfo {
	float delta_time;
	class Scene& scene;
	bool show_scene_info = false;
	std::shared_ptr<class MaterialManager> material_manager;
	bool show_material_info = false;
	GuiInfo(float delta_time_, Scene& scene_, const std::shared_ptr<MaterialManager>& material_manager_);
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