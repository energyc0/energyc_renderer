#include "RenderUnitSolid.h"
#include "RendererSolid.h"
#include "Camera.h"
#include "RenderManager.h"
#include "RendererGui.h"
#include <array>

RenderUnitSolid::RenderUnitSolid(const RenderUnitSolidCreateInfo& unit_create_info) :
	_camera(unit_create_info.camera),
	_depth_image(unit_create_info.depth_image),
	_depth_image_view(unit_create_info.depth_image_view){
	create_images();
	create_render_pass();
	create_framebuffers();
	create_descriptor_tools(unit_create_info);

	RendererSolidCreateInfo create_info{
		_render_pass,
		unit_create_info.global_UBO_descriptor_set_layout,
		unit_create_info.scene
	};
	_renderer_solid = new RendererSolid(create_info);
	LOG_STATUS("RenderUnitSolid - RendererSolid created.");

	_renderer_gui = new RendererGui(unit_create_info.window, _render_pass, unit_create_info.gui_info);
}

RenderUnitSolid::~RenderUnitSolid() {
	vkDestroyPipelineLayout(Core::get_device(), _pipeline_layout, nullptr);

	delete _image_views;
	delete _renderer_gui;
	delete _renderer_solid;
}

void RenderUnitSolid::create_images() {
	auto images = Core::get_swapchain_images();

	VulkanImageViewCreateInfo view_create_info{};
	view_create_info.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	view_create_info.layer_count = 1;
	view_create_info.mip_level_count = 1;
	view_create_info.type = VK_IMAGE_VIEW_TYPE_2D;

	_image_views = new VulkanMultipleImageViews(images, Core::get_swapchain_format(), view_create_info);
	LOG_STATUS("RenderUnitSolid - image views created.");

}

void RenderUnitSolid::create_render_pass() {
	std::array<VkAttachmentDescription, 2> attachments{};
	//color attachment
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments[0].format = Core::get_swapchain_format();
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;

	//depth attachment
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].format = _depth_image->get_format();
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	VkAttachmentReference color_attachment_reference{};
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_reference{};
	depth_attachment_reference.attachment = 1;
	depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass{};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_reference;
	subpass.pDepthStencilAttachment = &depth_attachment_reference;

	std::array<VkSubpassDependency,3> dependency{};
	dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency[0].dstSubpass = 0;
	dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[0].srcAccessMask = VK_ACCESS_NONE;
	dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependency[1].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency[1].dstSubpass = 0;
	dependency[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency[1].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency[1].srcAccessMask = VK_ACCESS_NONE;
	dependency[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependency[2].srcSubpass = 0;
	dependency[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependency[2].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[2].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[2].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[2].dstAccessMask = VK_ACCESS_NONE ;
	dependency[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	create_info.pSubpasses = &subpass;
	create_info.subpassCount = 1;
	create_info.attachmentCount = attachments.size();
	create_info.pAttachments = attachments.data();
	create_info.pDependencies = dependency.data();
	create_info.dependencyCount = dependency.size();

	VK_ASSERT(vkCreateRenderPass(Core::get_device(), &create_info, nullptr, &_render_pass), "vkCreateRenderPass(), RenderUnitSolid - FAILED");
	LOG_STATUS("Created RendererUnitSolid render pass.");
}

void RenderUnitSolid::create_framebuffers() {
	uint32_t image_count = Core::get_swapchain_image_count();
	std::vector<VkImageView> attachment_layout = { NULL, _depth_image_view->get_image_view()};
	std::vector<std::vector<VkImageView>> attachments;
	attachments.reserve(image_count);

	for (uint32_t i = 0; i < image_count; i++) {
		attachment_layout[0] = _image_views->get(i);
		attachments.push_back(attachment_layout);
	}

	_framebuffer = new VulkanMultipleFramebuffers(Core::get_swapchain_width(), Core::get_swapchain_height(), attachments, _render_pass);
	LOG_STATUS("RenderUnitSolid - framebuffers created.");
}

void RenderUnitSolid::create_descriptor_tools(const RenderUnitSolidCreateInfo& unit_create_info) {
	
	{
		VkPipelineLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.setLayoutCount = 1;
		create_info.pSetLayouts = &unit_create_info.global_UBO_descriptor_set_layout;
		VK_ASSERT(vkCreatePipelineLayout(Core::get_device(), &create_info, nullptr, &_pipeline_layout), "vkCreatePipelineLayout(), RenderUnitSolid - FAILED.");
		LOG_STATUS("Created RendererUnitSolid pipeline layout.");
	}
}

void RenderUnitSolid::fill_command_buffer(VkCommandBuffer command_buffer, const CurrentFrameData& frame_data) {
	std::array<VkClearValue,2> clear_values{};
	clear_values[0].color = {0.0f,0.0f,0.0f};
	clear_values[1].depthStencil = { 1.f, 0 };

	VkRenderPassBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	begin_info.clearValueCount = clear_values.size();
	begin_info.pClearValues = clear_values.data();
	begin_info.framebuffer = _framebuffer->get_framebuffer();
	begin_info.renderPass = _render_pass;
	begin_info.renderArea.offset = { 0,0 };
	begin_info.renderArea.extent = { _framebuffer->get_width(), _framebuffer->get_height() };

	vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
	
	VkViewport viewport{};
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = _framebuffer->get_width();
	viewport.height = _framebuffer->get_height();
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &begin_info.renderArea);

	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &frame_data.global_UBO, 0, 0);
	_renderer_solid->fill_command_buffer(command_buffer);
	_renderer_gui->fill_command_buffer(command_buffer);
	vkCmdEndRenderPass(command_buffer);
}