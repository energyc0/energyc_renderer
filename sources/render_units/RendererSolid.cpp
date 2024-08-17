#include "RenderUnitSolid.h"
#include "SceneObject.h"
#include "Scene.h"
#include <array>

const std::string vertex_shader_spv_path = std::string(RENDERER_DIRECTORY) + "/shaders/spir-v/solid_vert.spv";
const std::string fragment_shader_spv_path = std::string(RENDERER_DIRECTORY) + "/shaders/spir-v/solid_frag.spv";

RendererSolid::RendererSolid(const RendererSolidCreateInfo* create_info) : _scene(create_info->scene), _global_descriptor_sets(create_info->descriptor_sets) {
	create_descriptor_tools(create_info);
	create_pipeline(create_info);
}

void RendererSolid::create_descriptor_tools(const RendererSolidCreateInfo* renderer_create_info) {
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
		auto unit_bindings = RenderUnitSolid::get_bindings();
		auto model_bindings = Model::get_bindings();
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		bindings.reserve(unit_bindings.size() + model_bindings.size());
		for (auto& i : unit_bindings) {
			bindings.push_back(i);
		}
		for (auto& i : model_bindings) {
			bindings.push_back(i);
		}
		VkDescriptorSetLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = bindings.size();
		create_info.pBindings = bindings.data();
		VK_ASSERT(vkCreateDescriptorSetLayout(Core::get_device(), &create_info, nullptr, &_descriptor_set_layout), "vkCreateDescriptorSetLayout(), RendererSolid - FAILED");
	}

	{
		VkDescriptorSetLayout layouts[2] = { renderer_create_info->render_unit_set_layout, _scene.get_descriptor_set_layout() };
		VkPipelineLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.pSetLayouts = layouts;
		create_info.setLayoutCount = 2;
		VK_ASSERT(vkCreatePipelineLayout(Core::get_device(), &create_info, nullptr, &_pipeline_layout), "vkCreatePipelineLayout() RendererSolid - FAILED");
	}
}

void RendererSolid::create_pipeline(const RendererSolidCreateInfo* renderer_create_info) {
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

	auto color_blend_attachment = utils::set_pipeline_color_blend_attachment_state();
	std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments{ color_blend_attachment };
	auto color_blend = utils::set_pipeline_color_blend_state(color_blend_attachments);

	VkGraphicsPipelineCreateInfo create_info{};
	create_info.layout = _pipeline_layout;
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.subpass = 0;
	create_info.renderPass = renderer_create_info->render_pass;
	create_info.pStages = shader_stages.data();
	create_info.stageCount = shader_stages.size();
	
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

	vkDestroyShaderModule(Core::get_device(), vertex_shader, nullptr);
	vkDestroyShaderModule(Core::get_device(), fragment_shader, nullptr);
}

void RendererSolid::fill_command_buffer(VkCommandBuffer command_buffer) {
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline);
	_scene.draw(command_buffer, _pipeline_layout, _global_descriptor_sets[Core::get_current_frame()]);
}

RendererSolid::~RendererSolid() {}