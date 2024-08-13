#include "Core.h"
#include <fstream>

namespace utils {

	VkPipelineShaderStageCreateInfo set_pipeline_shader_stage(VkShaderModule shader, VkShaderStageFlagBits stage) {
		VkPipelineShaderStageCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		create_info.stage = stage;
		create_info.module = shader;
		create_info.pName = "main";
		return create_info;
	}

	VkPipelineInputAssemblyStateCreateInfo set_pipeline_input_assembly_state(VkPrimitiveTopology topology, VkBool32 primitive_restart_enable) {
		VkPipelineInputAssemblyStateCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		create_info.topology = topology;
		create_info.primitiveRestartEnable = primitive_restart_enable;
		return create_info;
	}

	VkPipelineVertexInputStateCreateInfo set_pipeline_vertex_input_state(std::vector<VkVertexInputAttributeDescription>& attribute, std::vector<VkVertexInputBindingDescription>& binding) {
		VkPipelineVertexInputStateCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		create_info.pVertexAttributeDescriptions = attribute.data();
		create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute.size());
		create_info.pVertexBindingDescriptions = binding.data();
		create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding.size());
		return create_info;
	}

	VkPipelineViewportStateCreateInfo set_pipeline_viewport_state(uint32_t viewport_count, uint32_t scissor_count) {
		VkPipelineViewportStateCreateInfo viewport_state{};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = viewport_count;
		viewport_state.scissorCount = scissor_count;
		return viewport_state;
	}

	VkPipelineRasterizationStateCreateInfo set_pipeline_rasterization_state(VkCullModeFlags cull_mode, VkPolygonMode polygon_mode, VkFrontFace front_face) {
		VkPipelineRasterizationStateCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		create_info.rasterizerDiscardEnable = VK_FALSE;
		create_info.polygonMode = polygon_mode;
		create_info.lineWidth = 1.0f;
		create_info.frontFace = front_face;
		create_info.cullMode = cull_mode;
		return create_info;
	}

	VkPipelineDynamicStateCreateInfo set_pipeline_dynamic_state(std::vector<VkDynamicState>& dynamic_states) {
		VkPipelineDynamicStateCreateInfo dynamic_state{};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state.pDynamicStates = dynamic_states.data();
		return dynamic_state;
	}

	VkPipelineMultisampleStateCreateInfo set_pipeline_multisample_state(VkSampleCountFlagBits samples) {
		VkPipelineMultisampleStateCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		create_info.rasterizationSamples = samples;
		return create_info;
	}

	VkPipelineColorBlendAttachmentState set_pipeline_color_blend_attachment_state() {
		VkPipelineColorBlendAttachmentState color_blend_attachment{};
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		return color_blend_attachment;
	}

	VkPipelineColorBlendStateCreateInfo set_pipeline_color_blend_state(const std::vector<VkPipelineColorBlendAttachmentState>& color_blend_attachments) {
		VkPipelineColorBlendStateCreateInfo color_blending{};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable = VK_FALSE;
		color_blending.logicOp = VK_LOGIC_OP_AND;
		color_blending.attachmentCount = color_blend_attachments.size();
		color_blending.pAttachments = color_blend_attachments.data();
		return color_blending;
	}

	VkPipelineDepthStencilStateCreateInfo set_pipeline_depth_stencil_state(VkBool32 depth_test_enable, VkBool32 depth_write_enable, VkCompareOp depth_compare_op) {
		VkPipelineDepthStencilStateCreateInfo depth_create{};
		depth_create.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_create.stencilTestEnable = VK_FALSE;
		depth_create.depthWriteEnable = depth_write_enable;
		depth_create.depthTestEnable = depth_test_enable;
		depth_create.minDepthBounds = 0.f;
		depth_create.maxDepthBounds = 1.f;
		depth_create.depthCompareOp = depth_compare_op;
		depth_create.back = {};
		depth_create.front = {};
		return depth_create;
	}

	VkShaderModule create_shader_module(const char* filename) {
		std::ifstream file;
		file.open(filename, std::ios_base::ate | std::ios_base::binary);
		if (!file.is_open()) {
			LOG_ERROR("Failed to open the file: ", filename);
		}
		const uint32_t size = file.tellg();
		std::vector<char> code(size);
		file.seekg(0);
		file.read(code.data(), size);
		file.close();
		VkShaderModule shader_module;
		VkShaderModuleCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());
		create_info.codeSize = static_cast<uint32_t>(code.size());
		VK_ASSERT(vkCreateShaderModule(Core::get_device(), &create_info, NULL, &shader_module), "vkCreateShaderModule() - FAILED.");
		return shader_module;
	}
}