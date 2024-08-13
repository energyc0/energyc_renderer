#include "RenderUnitSolid.h"
#include <array>

RenderUnitSolid::RenderUnitSolid() {
	create_render_pass();
	create_framebuffers();

	RendererSolidCreateInfo create_info{};
	create_info.render_objects_count = 1;
	create_info.render_pass = _render_pass;
	_renderer_solid = new RendererSolid(&create_info);
	LOG_STATUS("RenderUnitSolid - RendererSolid created.");
}

RenderUnitSolid::~RenderUnitSolid() {
	delete _image_views;
	delete _renderer_solid;
}

void RenderUnitSolid::create_render_pass() {
	VkAttachmentDescription color_attachment{};
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	color_attachment.format = Core::get_swapchain_format();
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference color_attachment_reference{};
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_reference;

	std::array<VkSubpassDependency,1> dependency{};
	dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency[0].dstSubpass = 0;
	dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[0].srcStageMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	create_info.pSubpasses = &subpass;
	create_info.subpassCount = 1;
	create_info.attachmentCount = 1;
	create_info.pAttachments = &color_attachment;
	create_info.pDependencies = dependency.data();
	create_info.dependencyCount = dependency.size();

	VK_ASSERT(vkCreateRenderPass(Core::get_device(), &create_info, nullptr, &_render_pass), "vkCreateRenderPass(), RenderUnitSolid - FAILED");
	LOG_STATUS("RenderUnitSolid - render pass created.");
}

void RenderUnitSolid::create_framebuffers() {
	auto images = Core::get_swapchain_images();

	_image_views = new VulkanMultipleImageViews(images, Core::get_swapchain_format(), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
	LOG_STATUS("RenderUnitSolid - image views created.");
	_framebuffer = new VulkanMultipleFramebuffers(Core::get_swapchain_width(), Core::get_swapchain_height(), _image_views->get_image_views(), _render_pass, images.size());
	LOG_STATUS("RenderUnitSolid - framebuffers created.");
}

void RenderUnitSolid::fill_command_buffer(VkCommandBuffer command_buffer) {

}