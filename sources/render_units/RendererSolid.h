#pragma once

#include "RendererBase.h"

struct RendererSolidCreateInfo {
	VkRenderPass render_pass;
	uint32_t render_objects_count;
};

class RendererSolid : public RendererBaseExt {
private:
	std::vector<VkDescriptorSet> _descriptor_sets;
private:
	void create_descriptor_tools(const RendererSolidCreateInfo* create_info);
	void create_pipeline(const RendererSolidCreateInfo* create_info);
public:
	RendererSolid(const RendererSolidCreateInfo* create_info);

	virtual void fill_command_buffer(VkCommandBuffer command_buffer);

	~RendererSolid();
};