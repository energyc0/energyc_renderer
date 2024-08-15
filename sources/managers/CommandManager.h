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

	static VkCommandBuffer begin_single_command_buffer() noexcept;
	static VkResult end_single_command_buffer(VkCommandBuffer command_buffer) noexcept;

	~CommandManager();
};