#include "RendererSolid.h"
#include "Camera.h"
#include "RenderManager.h"
#include "RendererLight.h"
#include "RendererGui.h"
#include "RenderUnitSolid.h"
#include <array>

RenderUnitSolid::RenderUnitSolid(const RenderUnitSolidCreateInfo& unit_create_info) :
	_camera(unit_create_info.camera),
	_depth_image(unit_create_info.depth_image),
	_depth_image_view(unit_create_info.depth_image_view),
	_hdr_image(unit_create_info.hdr_image),
	_hdr_image_view(unit_create_info.hdr_image_view),
	_bright_image(unit_create_info.bright_image){
	create_render_pass();
	create_framebuffers();
	create_descriptor_tools(unit_create_info);

	RendererSolidCreateInfo renderer_solid_create_info{
		_render_pass,
		unit_create_info.global_UBO_descriptor_set_layout,
		unit_create_info.material_manager,
		unit_create_info.scene
	};

	RendererLightSourceCreateInfo renderer_light_create_info{
		unit_create_info.scene,
		unit_create_info.global_UBO_descriptor_set_layout,
		_render_pass
	};

	_renderer_solid = new RendererSolid(renderer_solid_create_info);
	_renderer_light = new RendererLightSource(renderer_light_create_info);
	LOG_STATUS("Created RenderUnitSolid.");
}

RenderUnitSolid::~RenderUnitSolid() {
	vkDestroyPipelineLayout(Core::get_device(), _pipeline_layout, nullptr);

	delete _renderer_light;
	delete _renderer_solid;
}

void RenderUnitSolid::create_render_pass() {
	//write color to the hdr color attachment
	//write bright color to the bright hdr color attachment

	std::array<VkAttachmentDescription, 3> attachments{};
	//hdr color attachment
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[0].format = _hdr_image->get_format();
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

	//bright hdr color attachment
	attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachments[2].format = _bright_image->get_format();
	attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference color_attachment_reference{};
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_reference{};
	depth_attachment_reference.attachment = 1;
	depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference bright_color_attachment_reference{};
	bright_color_attachment_reference.attachment = 2;
	bright_color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkAttachmentReference references[] = { color_attachment_reference, bright_color_attachment_reference };
	VkSubpassDescription subpass{};
	subpass.colorAttachmentCount = 2;
	subpass.pColorAttachments = references;
	subpass.pDepthStencilAttachment = &depth_attachment_reference;

	std::array<VkSubpassDependency,3> dependency{};
	dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency[0].dstSubpass = 0;
	dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependency[1].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency[1].dstSubpass = 0;
	dependency[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency[1].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependency[2].srcSubpass = 0;
	dependency[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependency[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependency[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT ;
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
	LOG_STATUS("Created RenderUnitSolid render pass.");
}

void RenderUnitSolid::create_framebuffers() {
	std::vector<VkImageView> attachments = { 
		_hdr_image_view->get_image_view(),
		_depth_image_view->get_image_view(),
		_bright_image->get_image_view()
	};

	_framebuffer = new VulkanFramebuffer(Core::get_swapchain_width(), Core::get_swapchain_height(), attachments, _render_pass);
	LOG_STATUS("Created RenderUnitSolid framebuffers.");
}

void RenderUnitSolid::create_descriptor_tools(const RenderUnitSolidCreateInfo& unit_create_info) {
	
	{
		VkPushConstantRange push_range{};
		push_range.offset = 0;
		push_range.size = sizeof(glm::vec3);
		push_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkPipelineLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.setLayoutCount = 1;
		create_info.pSetLayouts = &unit_create_info.global_UBO_descriptor_set_layout;
		create_info.pPushConstantRanges = &push_range;
		create_info.pushConstantRangeCount = 1;
		VK_ASSERT(vkCreatePipelineLayout(Core::get_device(), &create_info, nullptr, &_pipeline_layout), "vkCreatePipelineLayout(), RenderUnitSolid - FAILED.");
		LOG_STATUS("Created RenderUnitSolid pipeline layout.");
	}
}

void RenderUnitSolid::fill_command_buffer(VkCommandBuffer command_buffer, const CurrentFrameData& frame_data) {
	std::array<VkClearValue,3> clear_values{};
	clear_values[0].color = {0.0f,0.0f,0.0f};
	clear_values[1].depthStencil = { 1.f, 0 };
	clear_values[2].color = { 0.0f,0.0f,0.0f };

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

	auto camera_pos = _camera.get_world_position();

	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &frame_data.global_UBO, 0, 0);
	vkCmdPushConstants(command_buffer, _pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::vec3), &camera_pos);
	_renderer_solid->fill_command_buffer(command_buffer);
	_renderer_light->fill_command_buffer(command_buffer);
	vkCmdEndRenderPass(command_buffer);
}