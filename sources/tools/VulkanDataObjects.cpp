#include "VulkanDataObjects.h"

int32_t VulkanDataObject::find_memory_type(uint32_t memory_type_bits, VkMemoryPropertyFlags memory_property) noexcept {
	VkPhysicalDeviceMemoryProperties phys_dev_memory_property;
	vkGetPhysicalDeviceMemoryProperties(Core::get_physical_device(), &phys_dev_memory_property);
	for (uint32_t memory_index = 0; memory_index < phys_dev_memory_property.memoryTypeCount; memory_index++) {
		
		uint32_t memory_type_bit = 1 << memory_index;
		const bool has_memory_type = memory_type_bit & memory_type_bits;

		VkMemoryPropertyFlags properties = phys_dev_memory_property.memoryTypes[memory_index].propertyFlags;
		const bool has_memory_properties = (properties & memory_property) == memory_property;

		if (has_memory_properties && has_memory_type) {
			return static_cast<uint32_t>(memory_index);
		}
	}
	return -1;
}

//
//
//VulkanFramebuffer
//
//
VkFramebuffer VulkanFramebufferBase::create_framebuffer(
	uint32_t width,
	uint32_t height,
	const std::vector<VkImageView>& attachments,
	VkRenderPass render_pass) noexcept {

	VkFramebuffer framebuffer;

	VkFramebufferCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	create_info.layers = 1;
	create_info.width = width;
	create_info.height = height;
	create_info.attachmentCount = attachments.size();
	create_info.pAttachments = attachments.data();
	create_info.renderPass = render_pass;

	VK_ASSERT(vkCreateFramebuffer(Core::get_device(), &create_info, nullptr, &framebuffer), "vkCreateFramebuffer() - FAILED");

	return framebuffer;
}

VulkanFramebuffer::VulkanFramebuffer(uint32_t width,
	uint32_t height,
	std::vector<VkImageView>& attachments,
	VkRenderPass render_pass) :
	VulkanFramebufferBase(width, height),
	_framebuffer(create_framebuffer(width, height, attachments, render_pass)) {}

VulkanFramebuffer::~VulkanFramebuffer() {
	vkDestroyFramebuffer(Core::get_device(), _framebuffer, nullptr);
}

VulkanMultipleFramebuffers::VulkanMultipleFramebuffers(uint32_t width,
	uint32_t height,
	const std::vector<VkImageView>& attachments,
	VkRenderPass render_pass,
	uint32_t framebuffers_count) :
	VulkanFramebufferBase(width, height) {

	assert((attachments.size() % framebuffers_count == 0));
	_framebuffers.resize(framebuffers_count);
	uint32_t framebuffer_attachments_size = attachments.size() / framebuffers_count;

	uint32_t idx = 0;
	for (auto& i : _framebuffers) {
		std::vector<VkImageView> framebuffer_attachments(framebuffer_attachments_size);
		for (uint32_t j = 0; j < framebuffer_attachments_size; j++) {
			framebuffer_attachments[j] = attachments[idx + j];
		}
		idx += framebuffer_attachments_size;
		i = create_framebuffer(width, height, framebuffer_attachments, render_pass);
	}
}

VulkanMultipleFramebuffers::~VulkanMultipleFramebuffers() {
	for (auto i : _framebuffers) {
		vkDestroyFramebuffer(Core::get_device(), i, nullptr);
	}
}

//
//
//VulkanImageView
//
//

VkImageView VulkanImageViewBase::create_image_view(
	VkImage image, VkFormat format, VkImageViewType type, VkImageAspectFlags aspect, uint32_t layer_count, uint32_t mip_level_count) {
	VkImageView image_view;
	
	VkImageViewCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = image;
	create_info.format = format;
	create_info.viewType = type;
	create_info.subresourceRange.aspectMask = aspect;
	create_info.subresourceRange.baseArrayLayer = 0;
	create_info.subresourceRange.baseMipLevel = 0;
	create_info.subresourceRange.layerCount = layer_count;
	create_info.subresourceRange.levelCount = mip_level_count;
	
	VK_ASSERT(vkCreateImageView(Core::get_device(), &create_info, nullptr, &image_view), "vkCreateImageView() - FAILED");

	return image_view;
}

VulkanMultipleImageViews::VulkanMultipleImageViews(
	const std::vector<VkImage>& images, VkFormat format, VkImageViewType type, VkImageAspectFlags aspect, uint32_t layer_count, uint32_t mip_level_count) {
	_image_views.resize(images.size());
	for (uint32_t i = 0; i < images.size(); i++) {
		_image_views[i] = create_image_view(images[i], format, type, aspect, layer_count, mip_level_count);
	}
}

VulkanMultipleImageViews::VulkanMultipleImageViews(
	const std::vector<VulkanImage>& vulkan_images, VkImageViewType type, VkImageAspectFlags aspect, uint32_t layer_count, uint32_t mip_level_count) {
	_image_views.resize(vulkan_images.size());
	for (uint32_t i = 0; i < vulkan_images.size(); i++) {
		_image_views[i] = create_image_view(vulkan_images[i].get_image(), vulkan_images[i].get_format(), type, aspect, layer_count, mip_level_count);
	}
}

VulkanMultipleImageViews::~VulkanMultipleImageViews() {
	for (auto& i : _image_views) {
		vkDestroyImageView(Core::get_device(), i, nullptr);
	}
}

VulkanImageView::VulkanImageView(VkImage image, VkFormat format, VkImageViewType type, VkImageAspectFlags aspect, uint32_t layer_count, uint32_t mip_level_count) :
	_image_view(create_image_view(image,format,type, aspect, layer_count, mip_level_count)) {}

VulkanImageView::VulkanImageView(const VulkanImage& vulkan_image, VkImageViewType type, VkImageAspectFlags aspect, uint32_t layer_count, uint32_t mip_level_count) :
	VulkanImageView(vulkan_image.get_image(), vulkan_image.get_format(), type, aspect, layer_count, mip_level_count) {}

VulkanImageView::~VulkanImageView() {
	vkDestroyImageView(Core::get_device(), _image_view, nullptr);
}

//
//
//VulkanBuffer
//
//

VulkanBuffer::VulkanBuffer(VkBufferUsageFlags usage,VkDeviceSize size, VkMemoryPropertyFlags memory_property) : _ptr(nullptr), _size(size){
	VkBufferCreateInfo buffer_create_info{};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.usage = usage;
	buffer_create_info.size = size;
	buffer_create_info.sharingMode = 
		Core::get_graphics_queue_family_index() == Core::get_present_queue_family_index() ?
		VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;

	uint32_t family_indices[] = { Core::get_graphics_queue_family_index(), Core::get_present_queue_family_index() };
	if (buffer_create_info.sharingMode == VK_SHARING_MODE_EXCLUSIVE) {
		buffer_create_info.pQueueFamilyIndices = nullptr;
		buffer_create_info.queueFamilyIndexCount = 0;
	}
	else {
		buffer_create_info.pQueueFamilyIndices = family_indices;
		buffer_create_info.queueFamilyIndexCount = 2;
	}
	VK_ASSERT(vkCreateBuffer(Core::get_device(), &buffer_create_info, nullptr, &_buffer), "vkCreateBuffer() - FAILED");

	VkMemoryRequirements memory_req;
	vkGetBufferMemoryRequirements(Core::get_device(), _buffer, &memory_req);
	
	int32_t memory_type_index = find_memory_type(memory_req.memoryTypeBits, memory_property);
	assert(memory_type_index != -1 && "Failed to find memory type.");
	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = memory_req.size;
	alloc_info.memoryTypeIndex = static_cast<uint32_t>(memory_type_index);
	VK_ASSERT(vkAllocateMemory(Core::get_device(), &alloc_info, nullptr, &_memory), "vkAllocateMemory() - FAILED");

	VK_ASSERT(vkBindBufferMemory(Core::get_device(), _buffer, _memory, 0), "vkBindBufferMemory() - FAILED");
}

void VulkanBuffer::copy_buffers(VkCommandBuffer command_buffer,
	const VulkanBuffer& src, const VulkanBuffer& dst,
	VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size) noexcept {
	VkBufferCopy copy{};
	copy.dstOffset = dst_offset;
	copy.srcOffset = src_offset;
	copy.size = size;
	vkCmdCopyBuffer(command_buffer, src._buffer, dst._buffer, 1, &copy);
}

VkDescriptorBufferInfo VulkanBuffer::get_info(VkDeviceSize offset, VkDeviceSize range) const noexcept {
	VkDescriptorBufferInfo info{};
	info.buffer = _buffer;
	info.offset = offset;
	info.range = range;
	return info;
}

VulkanBuffer::~VulkanBuffer() {
	if (_ptr != nullptr) {
		vkUnmapMemory(Core::get_device(), _memory);
	}
	vkFreeMemory(Core::get_device(), _memory, nullptr);
	vkDestroyBuffer(Core::get_device(), _buffer, nullptr);
}