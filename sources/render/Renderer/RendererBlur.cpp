#include "RendererBlur.h"
#include "VulkanDataObjects.h"

const std::string quad_shader_path = std::string(RENDERER_DIRECTORY) + "/shaders/spir-v/quad.spv";
const std::string quad_blur_path = std::string(RENDERER_DIRECTORY) + "/shaders/spir-v/quad_blur.spv";

RendererBlur::RendererBlur(const RendererBlurCreateInfo& create_info) {
	create_descriptor_tools(create_info);
	create_graphics_pipeline(create_info);
	LOG_STATUS("Created RendererBlur.");
}

RendererBlur::~RendererBlur() {
	vkDestroyDescriptorSetLayout(Core::get_device(), _descriptor_set_layout, nullptr);
}

void RendererBlur::create_graphics_pipeline(const RendererBlurCreateInfo& renderer_create_info) noexcept {
	VkShaderModule vertex_shader = utils::create_shader_module(quad_shader_path.c_str()),
		fragment_shader = utils::create_shader_module(quad_blur_path.c_str());

	VkPipelineShaderStageCreateInfo shader_stages[2]{
		utils::set_pipeline_shader_stage(vertex_shader,VK_SHADER_STAGE_VERTEX_BIT),
		utils::set_pipeline_shader_stage(fragment_shader,VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	auto input_assembly = utils::set_pipeline_input_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	std::vector<VkVertexInputAttributeDescription> attributes{};
	std::vector<VkVertexInputBindingDescription>bindings{};
	auto vertex_input = utils::set_pipeline_vertex_input_state(attributes, bindings);
	auto viewport = utils::set_pipeline_viewport_state(1, 1);
	std::vector<VkDynamicState> dynamic_states{ VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT };
	auto dynamic = utils::set_pipeline_dynamic_state(dynamic_states);
	auto rasterization = utils::set_pipeline_rasterization_state(VK_CULL_MODE_BACK_BIT);
	auto multisample = utils::set_pipeline_multisample_state();
	auto depth = utils::set_pipeline_depth_stencil_state(VK_FALSE, VK_FALSE);
	auto color_blend_attachment = utils::set_pipeline_color_blend_attachment_state();
	std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments{ color_blend_attachment };
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
	create_info.pDepthStencilState = &depth;
	create_info.pColorBlendState = &color_blend;

	VK_ASSERT(vkCreateGraphicsPipelines(Core::get_device(), nullptr, 1, &create_info, nullptr, &_graphics_pipeline), "vkCreateGraphicsPipelines() RendererBlur - FAILED");

	vkDestroyShaderModule(Core::get_device(), vertex_shader, nullptr);
	vkDestroyShaderModule(Core::get_device(), fragment_shader, nullptr);
}

void RendererBlur::create_descriptor_tools(const RendererBlurCreateInfo& renderer_create_info) noexcept{
	{
		auto binding = get_bindings();
		VkDescriptorSetLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = 1;
		create_info.pBindings = &binding;
		VK_ASSERT(vkCreateDescriptorSetLayout(Core::get_device(), &create_info, nullptr, &_descriptor_set_layout), "vkCreateDescriptorSetLayout(), RendererBlur - FAILED");
	}

	{
		VkDescriptorPoolSize pool_size{};
		pool_size.descriptorCount = 1;
		pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.pPoolSizes = &pool_size;
		create_info.poolSizeCount = 1;
		create_info.maxSets = 1;
		VK_ASSERT(vkCreateDescriptorPool(Core::get_device(), &create_info, nullptr, &_descriptor_pool), "vkCreateDescriptorPool(), RendererBlur - FAILED");

		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = _descriptor_pool;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &_descriptor_set_layout;
		VK_ASSERT(vkAllocateDescriptorSets(Core::get_device(), &alloc_info, &_descriptor_set), "vkAllocateDescriptorSets(), RendererBlur - FAILED");
		
		auto image_info = renderer_create_info.bright_image->get_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.dstArrayElement = 0;
		write.dstBinding = 0;
		write.dstSet = _descriptor_set;
		write.pImageInfo = &image_info;
		vkUpdateDescriptorSets(Core::get_device(), 1, &write, 0, 0);
	}

	{
		VkPipelineLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.setLayoutCount = 1;
		create_info.pSetLayouts = &_descriptor_set_layout;
		create_info.pPushConstantRanges = nullptr;
		create_info.pushConstantRangeCount = 0;
		VK_ASSERT(vkCreatePipelineLayout(Core::get_device(), &create_info, nullptr, &_pipeline_layout), "vkCreatePipelineLayout(), RendererBlur - FAILED.");
	}
}

VkDescriptorSetLayoutBinding RendererBlur::get_bindings() noexcept {

	VkDescriptorSetLayoutBinding binding{};
	binding.binding = 0;
	binding.descriptorCount = 1;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	return binding;
}

void RendererBlur::fill_command_buffer(VkCommandBuffer command_buffer) {
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline);
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &_descriptor_set, 0, 0);
	vkCmdDraw(command_buffer, 6, 1, 0, 0);
}