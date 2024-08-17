#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

#ifdef DEBUG
	#include <cassert>
	#define VK_ASSERT(expr, msg) assert(expr == VK_SUCCESS && msg)
#else
	#define VK_ASSERT(expr, msg) ((void)0)
#endif // DEBUG


namespace utils {

	class Log {
		template<typename T, typename... Args>
		static void message(T msg, Args... msgs) {
			std::cout << msg;
			message(msgs...);
		}
		template<typename T>
		static void message(T msg) {
			std::cout << msg << '\n';
		}
	public:
		template<typename T, typename... Args>
		static void status(T msg, Args... msgs) {
			std::cout << "---\t";
			message(msg, msgs...);
		}
		template<typename T, typename... Args>
		static void warning(T msg, Args... msgs) {
			std::cout << "!!!\t";
			message(msg, msgs...);
		}
		template<typename T, typename... Args>
		static void error(T msg, Args... msgs) {
			std::cout << "ERROR\t";
			message(msg, msgs...);
			exit(1);
		}
	};


	VkPipelineShaderStageCreateInfo set_pipeline_shader_stage(VkShaderModule shader, VkShaderStageFlagBits stage);
	VkPipelineInputAssemblyStateCreateInfo set_pipeline_input_assembly_state(VkPrimitiveTopology topology, VkBool32 primitive_restart_enable = VK_FALSE);
	VkPipelineVertexInputStateCreateInfo set_pipeline_vertex_input_state(std::vector<VkVertexInputAttributeDescription>& attribute, std::vector<VkVertexInputBindingDescription>& binding);
	VkPipelineViewportStateCreateInfo set_pipeline_viewport_state(uint32_t viewport_count, uint32_t scissor_count);
	VkPipelineRasterizationStateCreateInfo set_pipeline_rasterization_state(VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT, VkPolygonMode polygon_mode = VK_POLYGON_MODE_FILL, VkFrontFace front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE);
	VkPipelineDynamicStateCreateInfo set_pipeline_dynamic_state(std::vector<VkDynamicState>& dynamic_states);
	VkPipelineMultisampleStateCreateInfo set_pipeline_multisample_state(VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	VkPipelineColorBlendAttachmentState set_pipeline_color_blend_attachment_state();
	VkPipelineColorBlendStateCreateInfo set_pipeline_color_blend_state(const std::vector<VkPipelineColorBlendAttachmentState>& color_blend_attachments);
	VkPipelineDepthStencilStateCreateInfo set_pipeline_depth_stencil_state(VkBool32 depth_test_enable = VK_TRUE, VkBool32 depth_write_enable = VK_TRUE, VkCompareOp depth_compare_op = VK_COMPARE_OP_LESS);

	VkShaderModule create_shader_module(const char* filename);
}

#ifdef DEBUG
#define LOG_STATUS(msg, ...) utils::Log::status(msg, __VA_ARGS__)
#define LOG_WARNING(msg, ...) utils::Log::warning(msg, __VA_ARGS__)
#else
#define LOG_STATUS(msg) ((void)0)
#define LOG_WARNING(msg) ((void)0)
#endif // DEBUG

#define LOG_ERROR(msg, ...) utils::Log::error(msg, __VA_ARGS__)