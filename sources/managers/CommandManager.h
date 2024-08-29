#pragma once
#include "Utils.h"

class CommandManager {
private:
	std::vector<VkCommandBuffer> _command_buffers;
	VkCommandPool _command_pool;

	VkCommandBuffer _current_frame_cmd;

	std::vector<VkCommandBuffer> _dynamic_command_buffers;

	static CommandManager* cmd_manager_ptr;
public:
	CommandManager();

	void begin_frame_command_buffer() noexcept;
	inline VkCommandBuffer get_frame_command_buffer() const noexcept { return _current_frame_cmd; }
	inline void end_frame_command_buffer() const noexcept { VK_ASSERT(vkEndCommandBuffer(_current_frame_cmd), "vkEndCommandBuffer(), current frame - FAILED."); }

	static VkResult submit_queue(const std::vector<VkCommandBuffer>& cmd,
		const std::vector<VkSemaphore>& wait_semaphores,
		const std::vector<VkPipelineStageFlags>& wait_stage_flags,
		const std::vector<VkSemaphore>& signal_semaphores,
		VkFence fence) noexcept;

	static void copy_buffers(VkCommandBuffer command_buffer,
		const class VulkanBuffer& src, const class VulkanBuffer& dst,
		VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size) noexcept;
	
	static void copy_buffer_to_image(VkCommandBuffer command_buffer,
		const class VulkanBuffer& src_buffer, const class VulkanImage& dst_image,
		VkImageLayout dst_image_layout, const VkImageSubresourceLayers& subresource) noexcept;

	static void transition_image_layout(VkCommandBuffer command_buffer, const VulkanImage& image,
		VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
		VkAccessFlags src_access, VkAccessFlags dst_access,
		VkImageLayout old_layout, VkImageLayout new_layout,
		const VkImageSubresourceRange& subresource_range);

	static void set_memory_dependency(VkCommandBuffer command_buffer,
		VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
		VkAccessFlags src_access, VkAccessFlags dst_access) noexcept;

	static VkCommandBuffer begin_single_command_buffer() noexcept;
	static VkResult end_single_command_buffer(VkCommandBuffer command_buffer,
		const std::vector<VkSemaphore>& wait_semaphores = {},
		const std::vector<VkPipelineStageFlags>& wait_stage_flags = {},
		const std::vector<VkSemaphore>& signal_semaphores = {},
		VkFence fence = VK_NULL_HANDLE) noexcept;

	~CommandManager();

};