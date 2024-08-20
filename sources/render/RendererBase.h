#pragma once
#include "Core.h"

class RendererBase {
protected:
	VkDescriptorPool _descriptor_pool;

protected:
	RendererBase() :
		_descriptor_pool(VK_NULL_HANDLE) {}

public:

	virtual void fill_command_buffer(VkCommandBuffer command_buffer) = 0;

	virtual ~RendererBase() {
		vkDestroyDescriptorPool(Core::get_device(), _descriptor_pool, nullptr);
	}
};



class RendererBaseExt : public RendererBase {
protected:
	VkPipelineLayout _pipeline_layout;
	VkPipeline _graphics_pipeline;

protected:
	explicit RendererBaseExt() :
		_pipeline_layout(VK_NULL_HANDLE),
		_graphics_pipeline(VK_NULL_HANDLE) {}

public:

	virtual void fill_command_buffer(VkCommandBuffer command_buffer) = 0;

	virtual ~RendererBaseExt() {
		vkDestroyPipeline(Core::get_device(), _graphics_pipeline, nullptr);
		vkDestroyPipelineLayout(Core::get_device(), _pipeline_layout, nullptr);
	}
};