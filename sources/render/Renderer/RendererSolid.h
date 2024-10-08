#pragma once

#include "RendererBase.h"

struct RendererSolidCreateInfo {
	VkRenderPass render_pass;
	VkDescriptorSetLayout render_unit_set_layout;
	const std::shared_ptr<class MaterialManager>& material_manager;
	const std::shared_ptr<class Scene>& scene;
};

class RendererSolid : public RendererBaseExt {
private:
	const std::shared_ptr<class Scene>& _scene;
private:
	void create_descriptor_tools(const RendererSolidCreateInfo& create_info);
	void create_pipeline(const RendererSolidCreateInfo& create_info);
public:
	RendererSolid(const RendererSolidCreateInfo& create_info);

	virtual void fill_command_buffer(VkCommandBuffer command_buffer);

	~RendererSolid();
};