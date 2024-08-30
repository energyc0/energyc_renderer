#include "RendererBlur.h"
#include "RendererPostProcess.h"
#include "RendererGui.h"
#include "RenderUnitPostProcess.h"
#include <array>

RenderUnitPostProcess::RenderUnitPostProcess(const RenderUnitPostProcessCreateInfo& create_info) :
	_hdr_color_image(create_info.hdr_color_image),
	_hdr_color_image_view(create_info.hdr_color_image_view),
	_bright_color_image(create_info._bright_color_image),
	_staging_color_image(create_info.staging_color_image){
	create_images();
	create_render_pass();
	create_framebuffers();

	RendererBlurCreateInfo renderer_blur_create_info{
		_render_pass,
		create_info._bright_color_image
	};
	RendererPostProcessCreateInfo renderer_post_process_create_info{
		_render_pass,
		create_info.hdr_color_image_view,
		create_info.staging_color_image
	};

	_renderer_blur = std::unique_ptr<RendererBlur>(new RendererBlur(renderer_blur_create_info));
	_renderer_post_process = std::unique_ptr<RendererPostProcess>(new RendererPostProcess(renderer_post_process_create_info));
	_renderer_gui = std::unique_ptr< RendererGui>(new RendererGui(create_info.window, _render_pass, create_info.gui_info));

	LOG_STATUS("Created RenderUnitPostProcess.");
}

RenderUnitPostProcess::~RenderUnitPostProcess() {
	delete _image_views;
}

void RenderUnitPostProcess::create_render_pass() {
	//get color attachment, staging color attachment and hdr blur attachment
	//then write blurred color to the staging color attachment
	//post-process image and present it via input attachment

	std::array<VkAttachmentDescription, 4> attachments{};
	//present attachment
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments[0].format = Core::get_swapchain_format();
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;

	//color attachment
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[1].format = _hdr_color_image->get_format();
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	//hdr blur attachment
	attachments[2].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[2].format = _hdr_color_image->get_format();
	attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	//staging color attachment
	attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[3].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[3].format = _staging_color_image->get_format();
	attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	VkAttachmentReference present_attachment_reference{};
	present_attachment_reference.attachment = 0;
	present_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference input_color_attachment_reference{};
	input_color_attachment_reference.attachment = 1;
	input_color_attachment_reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference blur_attachment_reference{};
	blur_attachment_reference.attachment = 2;
	blur_attachment_reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference staging_color_attachment_reference{};
	staging_color_attachment_reference.attachment = 3;
	staging_color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference input_staging_color_attachment_reference{};
	input_staging_color_attachment_reference.attachment = 3;
	input_staging_color_attachment_reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	std::array<VkSubpassDescription,2> subpasses{};
	//blur image
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &staging_color_attachment_reference;

	VkAttachmentReference temp[2] = { input_color_attachment_reference ,input_staging_color_attachment_reference };
	//present image
	subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[1].colorAttachmentCount = 1;
	subpasses[1].pColorAttachments = &present_attachment_reference;
	subpasses[1].inputAttachmentCount = 2;
	subpasses[1].pInputAttachments = temp;

	std::array<VkSubpassDependency, 3> dependency{};
	dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency[0].dstSubpass = 0;
	dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependency[1].srcSubpass = 0;
	dependency[1].dstSubpass = 1;
	dependency[1].srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	dependency[1].dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	dependency[1].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
	dependency[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
	dependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependency[2].srcSubpass = 1;
	dependency[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependency[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[2].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[2].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	create_info.pSubpasses = subpasses.data();
	create_info.subpassCount = subpasses.size();
	create_info.attachmentCount = attachments.size();
	create_info.pAttachments = attachments.data();
	create_info.pDependencies = dependency.data();
	create_info.dependencyCount = dependency.size();

	VK_ASSERT(vkCreateRenderPass(Core::get_device(), &create_info, nullptr, &_render_pass), "vkCreateRenderPass(), RenderUnitPostProcess - FAILED");
}

void RenderUnitPostProcess::create_images() {
	auto images = Core::get_swapchain_images();

	VulkanImageViewCreateInfo view_create_info{};
	view_create_info.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	view_create_info.layer_count = 1;
	view_create_info.mip_level_count = 1;
	view_create_info.type = VK_IMAGE_VIEW_TYPE_2D;

	_image_views = new VulkanMultipleImageViews(images, Core::get_swapchain_format(), view_create_info);
}

void RenderUnitPostProcess::create_framebuffers() {
	uint32_t image_count = Core::get_swapchain_image_count();
	std::vector<VkImageView> attachment_layout = {
		NULL,
		_hdr_color_image_view->get_image_view(),
		_bright_color_image->get_image_view(),
		_staging_color_image->get_image_view()
	};
	std::vector<std::vector<VkImageView>> attachments;
	attachments.reserve(image_count);

	for (uint32_t i = 0; i < image_count; i++) {
		attachment_layout[0] = _image_views->get(i);
		attachments.push_back(attachment_layout);
	}

	_framebuffer = new VulkanMultipleFramebuffers(Core::get_swapchain_width(), Core::get_swapchain_height(), attachments, _render_pass);
}

void RenderUnitPostProcess::fill_command_buffer(VkCommandBuffer command_buffer, const struct CurrentFrameData& frame_data) {
	VkClearValue clear_value[4]{};
	clear_value[0].color = { 0.f,0.f,0.f };
	clear_value[1].color = { 0.f,0.f,0.f };
	clear_value[2].color = { 0.f,0.f,0.f };
	clear_value[3].color = { 0.f,0.f,0.f };
	VkRenderPassBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	begin_info.framebuffer = _framebuffer->get_framebuffer();
	begin_info.renderArea.offset = { 0,0 };
	begin_info.renderArea.extent = { Core::get_swapchain_width(), Core::get_swapchain_height() };
	begin_info.renderPass = _render_pass; 
	begin_info.clearValueCount = 4;
	begin_info.pClearValues = clear_value;
	vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);

	_renderer_blur->fill_command_buffer(command_buffer);

	vkCmdNextSubpass(command_buffer, VK_SUBPASS_CONTENTS_INLINE);

	_renderer_post_process->fill_command_buffer(command_buffer);
	_renderer_gui->fill_command_buffer(command_buffer);
	vkCmdEndRenderPass(command_buffer);
}