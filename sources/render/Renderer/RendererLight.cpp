#include "RendererLight.h"
#include "Scene.h"
#include <array>

const std::string vertex_shader_spv_path = std::string(RENDERER_DIRECTORY) + "/shaders/spir-v/light_source_vert.spv";
const std::string fragment_shader_spv_path = std::string(RENDERER_DIRECTORY) + "/shaders/spir-v/light_source_frag.spv";

RendererLightSource::RendererLightSource(const RendererLightSourceCreateInfo& renderer_create_info) :
	_scene(renderer_create_info.scene) {
	create_descriptor_tools(renderer_create_info);
	create_graphics_pipeline(renderer_create_info);
	LOG_STATUS("Created RendererLightSource.");
}

void RendererLightSource::fill_command_buffer(VkCommandBuffer command_buffer) {
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline);
	_scene->draw_light(command_buffer, _pipeline_layout);
}

void RendererLightSource::create_descriptor_tools(const RendererLightSourceCreateInfo& renderer_create_info) {
	VkPushConstantRange push_range{};
	push_range.offset = 0;
	push_range.size = sizeof(glm::vec3);
	push_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayout layouts[2] = {
		renderer_create_info.global_UBO_descriptor_set_layout,
		_scene->get_descriptor_set_layout()
	};
	VkPipelineLayoutCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	create_info.pSetLayouts = layouts;
	create_info.setLayoutCount = 2;
	create_info.pPushConstantRanges = &push_range;
	create_info.pushConstantRangeCount = 1;
	VK_ASSERT(vkCreatePipelineLayout(Core::get_device(), &create_info, nullptr, &_pipeline_layout), "vkCreatePipelineLayout() RendererSolid - FAILED");
	LOG_STATUS("Created RendererLightSource pipeline layout.");
}

void RendererLightSource::create_graphics_pipeline(const RendererLightSourceCreateInfo& renderer_create_info) {
	VkShaderModule vertex_shader = utils::create_shader_module(vertex_shader_spv_path.c_str()),
		fragment_shader = utils::create_shader_module(fragment_shader_spv_path.c_str());

	std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages{
		utils::set_pipeline_shader_stage(vertex_shader,VK_SHADER_STAGE_VERTEX_BIT),
		utils::set_pipeline_shader_stage(fragment_shader,VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	auto input_assembly = utils::set_pipeline_input_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	//empty vertex input
	std::vector<VkVertexInputAttributeDescription> attributes;
	std::vector<VkVertexInputBindingDescription>bindings;
	auto vertex_input = utils::set_pipeline_vertex_input_state(attributes, bindings);

	auto viewport = utils::set_pipeline_viewport_state(1, 1);
	std::vector<VkDynamicState> dynamic_states{ VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT };
	auto dynamic = utils::set_pipeline_dynamic_state(dynamic_states);
	auto rasterization = utils::set_pipeline_rasterization_state(VK_CULL_MODE_NONE);
	auto multisample = utils::set_pipeline_multisample_state();
	auto depth = utils::set_pipeline_depth_stencil_state();
	auto color_blend_attachment = utils::set_pipeline_color_blend_attachment_state();
	std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments(2,color_blend_attachment);
	auto color_blend = utils::set_pipeline_color_blend_state(color_blend_attachments);

	VkGraphicsPipelineCreateInfo create_info{};
	create_info.layout = _pipeline_layout;
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.subpass = 0;
	create_info.renderPass = renderer_create_info.render_pass;
	create_info.pStages = shader_stages.data();
	create_info.stageCount = shader_stages.size();

	create_info.pInputAssemblyState = &input_assembly;
	create_info.pVertexInputState = &vertex_input;
	create_info.pViewportState = &viewport;
	create_info.pTessellationState = nullptr;
	create_info.pDynamicState = &dynamic;
	create_info.pRasterizationState = &rasterization;
	create_info.pMultisampleState = &multisample;
	create_info.pDepthStencilState = &depth;
	create_info.pColorBlendState = &color_blend;

	VK_ASSERT(vkCreateGraphicsPipelines(Core::get_device(), nullptr, 1, &create_info, nullptr, &_graphics_pipeline), "vkCreateGraphicsPipelines() RendererLightSource - FAILED");
	LOG_STATUS("Created RendererLightSource graphics pipeline.");

	vkDestroyShaderModule(Core::get_device(), vertex_shader, nullptr);
	vkDestroyShaderModule(Core::get_device(), fragment_shader, nullptr);
}