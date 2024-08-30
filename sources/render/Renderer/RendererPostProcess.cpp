#include "RendererPostProcess.h"
#include "VulkanDataObjects.h"

const std::string quad_shader_path = std::string(RENDERER_DIRECTORY) + "/shaders/spir-v/quad.spv";
const std::string post_process_path = std::string(RENDERER_DIRECTORY) + "/shaders/spir-v/post_process.spv";

RendererPostProcess::RendererPostProcess(const RendererPostProcessCreateInfo& renderer_create_info) {
	create_descriptor_tools(renderer_create_info);
	create_graphics_pipeline(renderer_create_info);
	LOG_STATUS("Created RendererPostProcess.");
}

RendererPostProcess::~RendererPostProcess() {
	vkDestroyDescriptorSetLayout(Core::get_device(), _descriptor_set_layout, nullptr);
}

std::vector<VkDescriptorSetLayoutBinding> RendererPostProcess::get_bindings() noexcept{
	std::vector<VkDescriptorSetLayoutBinding> bindings(2);

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	return bindings;
}

void RendererPostProcess::create_descriptor_tools(const RendererPostProcessCreateInfo& renderer_create_info) {
	{
		auto bindings = get_bindings();
		VkDescriptorSetLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = bindings.size();
		create_info.pBindings = bindings.data();
		VK_ASSERT(vkCreateDescriptorSetLayout(Core::get_device(), &create_info, nullptr, &_descriptor_set_layout), "vkCreateDescriptorSetLayout(), RendererPostProcess - FAILED");
	}

	{
		VkDescriptorPoolSize pool_size{};
		pool_size.descriptorCount = 1;
		pool_size.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		VkDescriptorPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.pPoolSizes = &pool_size;
		create_info.poolSizeCount = 1;
		create_info.maxSets = 1;
		VK_ASSERT(vkCreateDescriptorPool(Core::get_device(), &create_info, nullptr, &_descriptor_pool), "vkCreateDescriptorPool(), RendererPostProcess - FAILED");

		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = _descriptor_pool;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &_descriptor_set_layout;
		VK_ASSERT(vkAllocateDescriptorSets(Core::get_device(), &alloc_info, &_descriptor_set), "vkAllocateDescriptorSets(), RendererPostProcess - FAILED");

		VkDescriptorImageInfo color_image_info{};
		color_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		color_image_info.imageView = renderer_create_info.hdr_color_image_view->get_image_view();
		color_image_info.sampler = VK_NULL_HANDLE;

		auto bright_color_image_info = renderer_create_info.staging_color_image->get_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkWriteDescriptorSet write[2]{};
		write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write[0].descriptorCount = 1;
		write[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		write[0].dstArrayElement = 0;
		write[0].dstBinding = 0;
		write[0].dstSet = _descriptor_set;
		write[0].pImageInfo = &color_image_info;

		write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write[1].descriptorCount = 1;
		write[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write[1].dstArrayElement = 0;
		write[1].dstBinding = 1;
		write[1].dstSet = _descriptor_set;
		write[1].pImageInfo = &bright_color_image_info;

		vkUpdateDescriptorSets(Core::get_device(), 2, write, 0, 0);
	}

	{
		VkPipelineLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.setLayoutCount = 1;
		create_info.pSetLayouts = &_descriptor_set_layout;
		create_info.pPushConstantRanges = nullptr;
		create_info.pushConstantRangeCount = 0;
		VK_ASSERT(vkCreatePipelineLayout(Core::get_device(), &create_info, nullptr, &_pipeline_layout), "vkCreatePipelineLayout(), RendererPostProcess - FAILED.");
	}
}

void RendererPostProcess::create_graphics_pipeline(const RendererPostProcessCreateInfo& renderer_create_info) noexcept {
	VkShaderModule vertex_shader = utils::create_shader_module(quad_shader_path.c_str()),
		fragment_shader = utils::create_shader_module(post_process_path.c_str());

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
	auto depth = utils::set_pipeline_depth_stencil_state(VK_FALSE,VK_FALSE);
	auto color_blend_attachment = utils::set_pipeline_color_blend_attachment_state();
	std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments{ color_blend_attachment };
	auto color_blend = utils::set_pipeline_color_blend_state(color_blend_attachments);

	VkGraphicsPipelineCreateInfo create_info{};
	create_info.layout = _pipeline_layout;
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.subpass = 0;
	create_info.renderPass = renderer_create_info.render_pass;
	create_info.subpass = 1;
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

void RendererPostProcess::fill_command_buffer(VkCommandBuffer command_buffer) {
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline);
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &_descriptor_set, 0, 0);
	vkCmdDraw(command_buffer, 6, 1, 0, 0);
}