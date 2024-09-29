#include "RendererEquirectangularProj.h"
#include "SceneObject.h"

const std::string vertex_shader_spv_path = std::string(RENDERER_DIRECTORY) + "/shaders/spir-v/equirectangular_projection_vert.spv";
const std::string fragment_shader_spv_path = std::string(RENDERER_DIRECTORY) + "/shaders/spir-v/equirectangular_projection_frag.spv";

RendererEquirectangularProj::RendererEquirectangularProj(const RendererEquirectangularProjCreateInfo& renderer_create_info) {
	create_descriptor_tools(renderer_create_info);
	create_graphics_pipeline(renderer_create_info);
}

void RendererEquirectangularProj::create_descriptor_tools(const RendererEquirectangularProjCreateInfo& renderer_create_info) {

}

void RendererEquirectangularProj::create_graphics_pipeline(const RendererEquirectangularProjCreateInfo& renderer_create_info) {
	VkShaderModule vertex_shader = utils::create_shader_module(vertex_shader_spv_path.c_str()),
		fragment_shader = utils::create_shader_module(fragment_shader_spv_path.c_str());

	VkPipelineShaderStageCreateInfo shader_stages[2]{
		utils::set_pipeline_shader_stage(vertex_shader,VK_SHADER_STAGE_VERTEX_BIT),
		utils::set_pipeline_shader_stage(fragment_shader,VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	auto input_assembly = utils::set_pipeline_input_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	std::vector<VkVertexInputAttributeDescription> attributes = Vertex::get_attribute_description();
	std::vector<VkVertexInputBindingDescription>bindings = Vertex::get_binding_description();
	auto vertex_input = utils::set_pipeline_vertex_input_state(attributes, bindings);
	auto viewport = utils::set_pipeline_viewport_state(1, 1);
	std::vector<VkDynamicState> dynamic_states{ VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT };
	auto dynamic = utils::set_pipeline_dynamic_state(dynamic_states);
	auto rasterization = utils::set_pipeline_rasterization_state(VK_CULL_MODE_BACK_BIT);
	auto multisample = utils::set_pipeline_multisample_state();
	auto color_blend_attachment = utils::set_pipeline_color_blend_attachment_state();
	std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments(2, color_blend_attachment);
	auto color_blend = utils::set_pipeline_color_blend_state(color_blend_attachments);

	VkGraphicsPipelineCreateInfo create_info{};
	create_info.layout = _pipeline_layout;
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.subpass = 0;
	create_info.renderPass = renderer_create_info.render_pass;
	create_info.pStages = shader_stages;
	create_info.stageCount = 2;

	create_info.pInputAssemblyState = &input_assembly;
	create_info.pVertexInputState = &vertex_input;
	create_info.pViewportState = &viewport;
	create_info.pTessellationState = nullptr;
	create_info.pDynamicState = &dynamic;
	create_info.pRasterizationState = &rasterization;
	create_info.pMultisampleState = &multisample;
	create_info.pDepthStencilState = nullptr;
	create_info.pColorBlendState = &color_blend;

	VK_ASSERT(vkCreateGraphicsPipelines(Core::get_device(), nullptr, 1, &create_info, nullptr, &_graphics_pipeline), "vkCreateGraphicsPipelines() RendererSolid - FAILED");
	LOG_STATUS("Created RendererSolid graphics pipeline.");

	vkDestroyShaderModule(Core::get_device(), vertex_shader, nullptr);
	vkDestroyShaderModule(Core::get_device(), fragment_shader, nullptr);
}

void RendererEquirectangularProj::fill_command_buffer(VkCommandBuffer command_buffer) {

}

RendererEquirectangularProj::~RendererEquirectangularProj() {

}