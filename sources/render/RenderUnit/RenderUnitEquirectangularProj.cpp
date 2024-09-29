#include "RenderUnitEquirectangularProj.h"
#include <array>

RenderUnitEquirectangularProj::RenderUnitEquirectangularProj(const RenderUnitEquirectangularProjCreateInfo& create_info) : 
	_equirectangular_env_map(create_info.equirectangular_env_map),
	_skybox(create_info.skybox){
	create_render_pass();
	create_framebuffers();

	RendererEquirectangularProjCreateInfo renderer_create_info{
		_render_pass,
		create_info.equirectangular_env_map
	};

	_renderer_proj = std::unique_ptr<RendererEquirectangularProj>(new RendererEquirectangularProj(renderer_create_info));
}

void RenderUnitEquirectangularProj::create_render_pass() {
	//get a equirectangular projection image and create a skybpx

	std::array<VkAttachmentDescription, 2> attachments{};
	//equirectangular projection
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[0].format = _equirectangular_env_map->get_format();
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;

	//skybox
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[1].format = _skybox->get_format();
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference projection_attachment_reference{};
	projection_attachment_reference.attachment = 0;
	projection_attachment_reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference skybox_attachment_reference{};
	skybox_attachment_reference.attachment = 1;
	skybox_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &skybox_attachment_reference;

	std::array<VkSubpassDependency, 2> dependency{};
	dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency[0].dstSubpass = 0;
	dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependency[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependency[1].srcSubpass = 0;
	dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	create_info.pSubpasses = &subpass;
	create_info.subpassCount = 1;
	create_info.attachmentCount = attachments.size();
	create_info.pAttachments = attachments.data();
	create_info.pDependencies = dependency.data();
	create_info.dependencyCount = dependency.size();

	VK_ASSERT(vkCreateRenderPass(Core::get_device(), &create_info, nullptr, &_render_pass), "vkCreateRenderPass(), RenderUnitEquirectangularProj - FAILED");

}

void RenderUnitEquirectangularProj::create_framebuffers() {
	std::vector<VkImageView> attachments = {
		_equirectangular_env_map->get_image_view(),
		_skybox->get_image_view(),
	};

	_framebuffer = new VulkanFramebuffer(Core::get_swapchain_width(), Core::get_swapchain_height(), attachments, _render_pass);
}

void RenderUnitEquirectangularProj::fill_command_buffer(VkCommandBuffer command_buffer, const struct CurrentFrameData& frame_data) {

	VkRenderPassBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	begin_info.framebuffer = _framebuffer->get_framebuffer();
	begin_info.renderPass = _render_pass;
	begin_info.renderArea.offset = { 0,0 };
	begin_info.renderArea.extent = { _skybox->get_width(), _skybox->get_height() };
	begin_info.clearValueCount = 2;
	VkClearValue clear_values[2]{};
	clear_values[0].color = { 0.f,0.f,0.f };
	clear_values[1].color = { 0.f,0.f,0.f };
	begin_info.pClearValues = clear_values;

	vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
	
	_renderer_proj->fill_command_buffer(command_buffer);

	vkCmdEndRenderPass(command_buffer);
}

RenderUnitEquirectangularProj::~RenderUnitEquirectangularProj() {

}