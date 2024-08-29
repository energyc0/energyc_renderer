#include "CommandManager.h"
#include "VulkanDataObjects.h"
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

void CommandManager::copy_buffers(VkCommandBuffer command_buffer,
	const VulkanBuffer& src, const VulkanBuffer& dst,
	VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size) noexcept {
	VkBufferCopy copy{};
	copy.dstOffset = dst_offset;
	copy.srcOffset = src_offset;
	copy.size = size;
	vkCmdCopyBuffer(command_buffer, src._buffer, dst._buffer, 1, &copy);
}

void CommandManager::copy_buffer_to_image(VkCommandBuffer command_buffer,
	const VulkanBuffer& src_buffer, const VulkanImage& dst_image,
	VkImageLayout dst_image_layout, const VkImageSubresourceLayers& subresource) noexcept {

	VkBufferImageCopy region{};
	region.bufferImageHeight = dst_image.get_height();
	region.bufferOffset = 0;
	region.bufferRowLength = dst_image.get_width();
	region.imageOffset = { 0,0,0 };
	region.imageExtent = { dst_image.get_width(), dst_image.get_height(), 1 };
	region.imageSubresource = subresource;
	vkCmdCopyBufferToImage(command_buffer, src_buffer._buffer, dst_image.get_image(), dst_image_layout, 1, &region);
}

void CommandManager::transition_image_layout(VkCommandBuffer command_buffer, const VulkanImage& image,
	VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
	VkAccessFlags src_access, VkAccessFlags dst_access,
	VkImageLayout old_layout, VkImageLayout new_layout,
	const VkImageSubresourceRange& subresource_range) {
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = src_access;
	barrier.dstAccessMask = dst_access;
	barrier.image = image.get_image();
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.subresourceRange = subresource_range;
	barrier.srcQueueFamilyIndex = Core::get_graphics_queue_family_index();
	barrier.dstQueueFamilyIndex = Core::get_graphics_queue_family_index();
	vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, 0, 0, 0, 1, &barrier);
}

void CommandManager::set_memory_dependency(VkCommandBuffer command_buffer,
	VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
	VkAccessFlags src_access, VkAccessFlags dst_access) noexcept {

	VkMemoryBarrier memory_barrier{};
	memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memory_barrier.srcAccessMask = src_access;
	memory_barrier.dstAccessMask = dst_access;
	vkCmdPipelineBarrier(command_buffer,
		src_stage, dst_stage,
		NULL,
		1, &memory_barrier, 0, 0, 0, 0);
}

VkCommandBuffer CommandManager::begin_single_command_buffer() noexcept {
	VkCommandBuffer command_buffer;
	cmd_manager_ptr->_dynamic_command_buffers.push_back(command_buffer);

	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandBufferCount = 1;
	alloc_info.commandPool = cmd_manager_ptr->_command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VK_ASSERT(vkAllocateCommandBuffers(Core::get_device(), &alloc_info, &command_buffer), "vkAllocateCommandBuffers() - FAILED");

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(command_buffer, &begin_info);

	return command_buffer;
}

VkResult CommandManager::end_single_command_buffer(VkCommandBuffer command_buffer,
	const std::vector<VkSemaphore>& wait_semaphores,
	const std::vector<VkPipelineStageFlags>& wait_stage_flags,
	const std::vector<VkSemaphore>& signal_semaphores,
	VkFence fence) noexcept {
	VkResult result = vkEndCommandBuffer(command_buffer);

	if (result != VK_SUCCESS)
		return result;

	result = submit_queue({ command_buffer }, wait_semaphores, wait_stage_flags, signal_semaphores, fence);
	return result;
}

CommandManager::~CommandManager() {
	vkDestroyCommandPool(Core::get_device(), _command_pool, nullptr);
}