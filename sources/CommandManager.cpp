#include "CommandManager.h"
#include "Core.h"

CommandManager* CommandManager::cmd_manager_ptr = nullptr;

CommandManager::CommandManager() : _current_frame_cmd(VK_NULL_HANDLE) {
	assert(cmd_manager_ptr == nullptr && "There can be only one CommandManager.");

	cmd_manager_ptr = this;

	VkCommandPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.queueFamilyIndex = Core::get_graphics_queue_family_index();
	create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_ASSERT(vkCreateCommandPool(Core::get_device(), &create_info, nullptr, &_command_pool), "vkCreateCommandPool() - FAILED");
	LOG_STATUS("Created command pool.");

	_command_buffers.resize(Core::get_swapchain_image_count());
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandBufferCount = _command_buffers.size();
	alloc_info.commandPool = _command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VK_ASSERT(vkAllocateCommandBuffers(Core::get_device(), &alloc_info, _command_buffers.data()), "vkAllocateCommandBuffers() - FAILED");
	LOG_STATUS("Allocated command buffers.");
}

void CommandManager::begin_frame_command_buffer() noexcept {

	VkCommandBufferBeginInfo begin_info{};
	_current_frame_cmd = _command_buffers[Core::get_current_frame()];

	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(_current_frame_cmd, &begin_info);
}

VkResult CommandManager::submit_queue(const std::vector<VkCommandBuffer>& cmd,
	const std::vector<VkSemaphore>& wait_semaphores,
	const std::vector<VkPipelineStageFlags>& wait_stage_flags,
	const std::vector<VkSemaphore>& signal_semaphores,
	VkFence fence) noexcept{
	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = cmd.size();
	submit.pCommandBuffers = cmd.data();
	submit.waitSemaphoreCount = wait_semaphores.size();
	submit.pWaitSemaphores = wait_semaphores.data();
	submit.pWaitDstStageMask = wait_stage_flags.data();
	submit.signalSemaphoreCount = signal_semaphores.size();
	submit.pSignalSemaphores = signal_semaphores.data();
	return vkQueueSubmit(Core::get_graphics_queue(), 1, &submit, fence);
}

CommandManager::~CommandManager() {
	vkDestroyCommandPool(Core::get_device(), _command_pool, nullptr);
}