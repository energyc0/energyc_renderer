#include "RendererSolid.h"
#include "RenderUnitSolid.h"
#include "SceneObject.h"
#include "Scene.h"
#include "RenderManager.h"
#include "RendererGui.h"
#include "MaterialManager.h"
#include <array>

const std::string vertex_shader_spv_path = std::string(RENDERER_DIRECTORY) + "/shaders/spir-v/solid_vert.spv";
const std::string fragment_shader_spv_path = std::string(RENDERER_DIRECTORY) + "/shaders/spir-v/solid_frag.spv";

RendererSolid::RendererSolid(const RendererSolidCreateInfo& create_info) : _scene(create_info.scene){
	create_descriptor_tools(create_info);
	create_pipeline(create_info);
	LOG_STATUS("Created RendererSolid.");
}

void RendererSolid::create_descriptor_tools(const RendererSolidCreateInfo& renderer_create_info) {
	{
		std::array<VkDescriptorPoolSize, 1> pool_sizes{};
		pool_sizes[0].descriptorCount = 1;
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkDescriptorPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.pPoolSizes = pool_sizes.data();
		create_info.poolSizeCount = pool_sizes.size();
		create_info.maxSets = 1;
		VK_ASSERT(vkCreateDescriptorPool(Core::get_device(), &create_info, nullptr, &_descriptor_pool), "vkCreateDescriptorPool(), RendererSolid - FAILED");
		LOG_STATUS("Created RendererSolid descriptor pool.");
	}

	{
		VkPushConstantRange push_range{};
		push_range.offset = 0;
		push_range.size = sizeof(glm::vec3);
		push_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayout layouts[3] = { 
			renderer_create_info.render_unit_set_layout,
			_scene->get_descriptor_set_layout(),
			renderer_create_info.material_manager->get_descriptor_set_layout()
		};
		VkPipelineLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.pSetLayouts = layouts;
		create_info.setLayoutCount = 3;
		create_info.pPushConstantRanges = &push_range;
		create_info.pushConstantRangeCount = 1;
		VK_ASSERT(vkCreatePipelineLayout(Core::get_device(), &create_info, nullptr, &_pipeline_layout), "vkCreatePipelineLayout() RendererSolid - FAILED");
		LOG_STATUS("Created RendererSolid pipeline layout.");
	}
}

void RendererSolid::create_pipeline(const RendererSolidCreateInfo& renderer_create_info) {
	VkShaderModule vertex_shader = utils::create_shader_module(vertex_shader_spv_path.c_str()),
		fragment_shader = utils::create_shader_module(fragment_shader_spv_path.c_str());

	std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages{
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
	auto depth = utils::set_pipeline_depth_stencil_state();
	auto color_blend_attachment = utils::set_pipeline_color_blend_attachment_state();
	std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments{ color_blend_attachment };
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

	VK_ASSERT(vkCreateGraphicsPipelines(Core::get_device(), nullptr, 1, &create_info, nullptr, &_graphics_pipeline), "vkCreateGraphicsPipelines() RendererSolid - FAILED");
	LOG_STATUS("Created RendererSolid graphics pipeline.");

	vkDestroyShaderModule(Core::get_device(), vertex_shader, nullptr);
	vkDestroyShaderModule(Core::get_device(), fragment_shader, nullptr);
}

void RendererSolid::fill_command_buffer(VkCommandBuffer command_buffer) {
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline);
	_scene->draw_solid(command_buffer, _pipeline_layout);
}

RendererSolid::~RendererSolid() {}