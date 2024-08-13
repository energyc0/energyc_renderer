#pragma once

#include "VulkanDataObjects.h"

class RenderUnitBase {
protected:
	VkRenderPass _render_pass;
	VulkanFramebufferBase* _framebuffer;

protected:
	virtual void create_render_pass() = 0;
	virtual void create_framebuffers() = 0;

	RenderUnitBase() : _render_pass(VK_NULL_HANDLE), _framebuffer(nullptr) {}

public:

	//begin and end render pass
	virtual void fill_command_buffer(VkCommandBuffer command_buffer) = 0;

	inline VkRenderPass get_render_pass() const noexcept { return _render_pass; }

	virtual ~RenderUnitBase() {
		delete _framebuffer;
		vkDestroyRenderPass(Core::get_device(), _render_pass, nullptr);
	}
};