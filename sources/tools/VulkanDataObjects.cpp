#include "VulkanDataObjects.h"
#include "CommandManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
	VkRenderPass render_pass) noexcept :
	VulkanFramebufferBase(width, height),
	_framebuffer(create_framebuffer(width, height, attachments, render_pass)) {}

VulkanFramebuffer::~VulkanFramebuffer() {
	vkDestroyFramebuffer(Core::get_device(), _framebuffer, nullptr);
}

VulkanMultipleFramebuffers::VulkanMultipleFramebuffers(uint32_t width,
	uint32_t height,
	const std::vector<std::vector<VkImageView>>& attachments,
	VkRenderPass render_pass) noexcept :
	VulkanFramebufferBase(width, height) {

	_framebuffers.resize(attachments.size());

	for (uint32_t i = 0; i < _framebuffers.size(); i++) {
		_framebuffers[i] = create_framebuffer(width, height, attachments[i], render_pass);
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
	VkImage image, VkFormat format, const VulkanImageViewCreateInfo& view_create_info) {
	VkImageView image_view;
	
	VkImageViewCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = image;
	create_info.format = format;
	create_info.viewType = view_create_info.type;
	create_info.subresourceRange.aspectMask = view_create_info.aspect;
	create_info.subresourceRange.baseArrayLayer = 0;
	create_info.subresourceRange.baseMipLevel = 0;
	create_info.subresourceRange.layerCount = view_create_info.layer_count;
	create_info.subresourceRange.levelCount = view_create_info.mip_level_count;
	
	VK_ASSERT(vkCreateImageView(Core::get_device(), &create_info, nullptr, &image_view), "vkCreateImageView() - FAILED");

	return image_view;
}

VulkanMultipleImageViews::VulkanMultipleImageViews(
	const std::vector<VkImage>& images, VkFormat format, const VulkanImageViewCreateInfo& view_create_info) noexcept {
	_image_views.resize(images.size());
	for (uint32_t i = 0; i < images.size(); i++) {
		_image_views[i] = create_image_view(images[i], format, view_create_info);
	}
}

VulkanMultipleImageViews::VulkanMultipleImageViews(
	const std::vector<VulkanImage>& vulkan_images, const VulkanImageViewCreateInfo& view_create_info) noexcept {
	_image_views.resize(vulkan_images.size());
	for (uint32_t i = 0; i < vulkan_images.size(); i++) {
		_image_views[i] = create_image_view(vulkan_images[i].get_image(), vulkan_images[i].get_format(), view_create_info);
	}
}

VulkanMultipleImageViews::VulkanMultipleImageViews(VulkanMultipleImageViews&& image_views) noexcept : _image_views(std::move(image_views._image_views)){}

VulkanMultipleImageViews::~VulkanMultipleImageViews() {
	for (auto& i : _image_views) {
		vkDestroyImageView(Core::get_device(), i, nullptr);
	}
}

VulkanImageView::VulkanImageView(VkImage image, VkFormat format, const VulkanImageViewCreateInfo& view_create_info) noexcept :
	_image_view(create_image_view(image,format, view_create_info)) {}

VulkanImageView::VulkanImageView(const VulkanImage& vulkan_image, const VulkanImageViewCreateInfo& view_create_info) noexcept :
	VulkanImageView(vulkan_image.get_image(), vulkan_image.get_format(), view_create_info) {}

VulkanImageView::VulkanImageView(VulkanImageView&& image_view) noexcept : _image_view(image_view._image_view) {}

VulkanImageView::~VulkanImageView() {
	vkDestroyImageView(Core::get_device(), _image_view, nullptr);
}

VulkanImageView::VulkanImageView() noexcept : _image_view(VK_NULL_HANDLE) {}

//
//
//VulkanBuffer
//
//

VulkanBuffer::VulkanBuffer(VkBufferUsageFlags usage,VkDeviceSize size, VkMemoryPropertyFlags memory_property) noexcept : _ptr(nullptr), _size(size){
	create_vulkan_buffer(usage, size, memory_property);
}

VkDescriptorBufferInfo VulkanBuffer::get_info(VkDeviceSize offset, VkDeviceSize range) const noexcept {
	VkDescriptorBufferInfo info{};
	info.buffer = _buffer;
	info.offset = offset;
	info.range = range;
	return info;
}

void VulkanBuffer::recreate(VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memory_property) noexcept {
	free_vulkan_buffer();
	create_vulkan_buffer(usage, size, memory_property);
}

void VulkanBuffer::create_vulkan_buffer(VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memory_property) noexcept {
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

void VulkanBuffer::free_vulkan_buffer() noexcept {
	if (_ptr != nullptr) {
		vkUnmapMemory(Core::get_device(), _memory);
	}
	vkFreeMemory(Core::get_device(), _memory, nullptr);
	vkDestroyBuffer(Core::get_device(), _buffer, nullptr);
}

VulkanBuffer::~VulkanBuffer() {
	free_vulkan_buffer();
}

//
//
//StagingBuffer
//
//
//

constexpr VkDeviceSize FIRST_STAGING_BUFFER_ALLOCATION = 65536;
VkDeviceSize StagingBuffer::_last_copied_size = FIRST_STAGING_BUFFER_ALLOCATION;
StagingBuffer* StagingBuffer::ptr = nullptr;

StagingBuffer::StagingBuffer() : _buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	_last_copied_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {

	assert(ptr == nullptr && "There can only be one instance of staging buffer.");

	ptr = this;
}

void StagingBuffer::copy_buffers(VkCommandBuffer command_buffer, const class VulkanBuffer& dst,
	VkDeviceSize src_offset, VkDeviceSize dst_offset) noexcept {
	CommandManager::copy_buffers(command_buffer, ptr->_buffer, dst, src_offset, dst_offset, _last_copied_size);
}

void StagingBuffer::copy_buffer_to_image(VkCommandBuffer command_buffer, const class VulkanImage& dst_image,
	VkImageLayout dst_image_layout, const VkImageSubresourceLayers& subresource) noexcept {
	CommandManager::copy_buffer_to_image(command_buffer, ptr->_buffer, dst_image, dst_image_layout, subresource);
}

void StagingBuffer::recreate_buffer() noexcept {
	ptr->_buffer.recreate(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, _last_copied_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

void StagingBuffer::copy_data_to_buffer(const void* data, size_t size) {
	_last_copied_size = size;

	if (_last_copied_size > ptr->_buffer.get_size()) {
		recreate_buffer();
	}

	char* data_ptr = ptr->_buffer.map_memory(0, _last_copied_size);
	memcpy(data_ptr, data, size);
	ptr->_buffer.unmap_memory();
}

//
//
//VulkanImage
//
//

VulkanImage::VulkanImage(const VulkanImageCreateInfo& image_create_info) noexcept :
	VulkanResizable(image_create_info.width, image_create_info.height),
	_format(image_create_info.format),
	_image(create_image(image_create_info)) {}

VulkanImage::VulkanImage(VulkanImage&& image) noexcept : VulkanResizable(image._width, image._height),
	_image(image._image), _memory(image._memory), _format(image._format) {}

VkImage VulkanImage::create_image(const VulkanImageCreateInfo& image_create_info) {
	VkImage image;
	VkImageCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.extent.depth = 1.f;
	create_info.extent.width = image_create_info.width;
	create_info.extent.height = image_create_info.height;
	create_info.arrayLayers = image_create_info.array_layers;
	create_info.format = image_create_info.format;
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_info.mipLevels = image_create_info.mip_levels;
	create_info.samples = image_create_info.samples;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.usage = image_create_info.usage;
	VK_ASSERT(vkCreateImage(Core::get_device(), &create_info, nullptr, &image), "vkCreateImage() - FAILED");

	VkMemoryRequirements requirements;
	vkGetImageMemoryRequirements(Core::get_device(), image, &requirements);

	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = requirements.size;
	alloc_info.memoryTypeIndex = find_memory_type(requirements.memoryTypeBits, image_create_info.memory_property);
	VK_ASSERT(vkAllocateMemory(Core::get_device(), &alloc_info, nullptr, &_memory), "vkAllocateMemory() - FAILED");

	VK_ASSERT(vkBindImageMemory(Core::get_device(), image, _memory, 0), "vkBindImageMemory() - FAILED");

	return image;
}

VulkanImage::VulkanImage(VkFormat format) noexcept : VulkanResizable(0,0), _format(format), _image(VK_NULL_HANDLE), _memory(VK_NULL_HANDLE) {}

VulkanImage::~VulkanImage() {
	vkFreeMemory(Core::get_device(), _memory, nullptr);
	vkDestroyImage(Core::get_device(), _image, nullptr);
}

//
//
//VulkanTexture
//
//

VulkanTextureBase::VulkanTextureBase(VkFormat format) noexcept : VulkanImage(format), _sampler(VK_NULL_HANDLE) {}

VulkanTextureBase::VulkanTextureBase(const VulkanTextureBase& texture) noexcept : _sampler(texture._sampler), VulkanImage(texture._format) {
	_image = texture._image;
	_image_view = texture._image_view;
	_memory = texture._memory;
	_width = texture._width;
	_height = texture._height;
}

VulkanTextureBase::VulkanTextureBase(VulkanTextureBase&& texture) noexcept : VulkanTextureBase(texture) {}

VulkanTextureBase::~VulkanTextureBase(){
	vkDestroySampler(Core::get_device(), _sampler, nullptr);
}

VulkanTexture2D::VulkanTexture2D() : VulkanTextureBase(VK_FORMAT_UNDEFINED) {}

VulkanTexture2D::VulkanTexture2D(VkFormat format, const char* filename) noexcept : VulkanTextureBase(format){
	load_texture(filename);
}

VulkanTexture2D::VulkanTexture2D(VulkanTexture2D&& texture) noexcept : VulkanTextureBase(texture){}

VkDescriptorImageInfo VulkanTexture2D::get_info(VkImageLayout layout) const noexcept {
	VkDescriptorImageInfo info{};
	info.imageLayout = layout;
	info.imageView = _image_view;
	info.sampler = _sampler;

	return info;
}

void VulkanTexture2D::load_texture(const char* filename) {
	int width, height, comp;
	auto pixels = stbi_load(filename, &width, &height, &comp, STBI_rgb_alpha);

	if (!pixels) {
		LOG_ERROR("Failed to load the image: ", filename);
	}

	LOG_STATUS("Loaded ", filename);

	_width = width;
	_height = height;

	VulkanImageCreateInfo image_create_info{};
	image_create_info.width = width;
	image_create_info.height = height;
	image_create_info.mip_levels = 1;
	image_create_info.memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	image_create_info.array_layers = 1;
	image_create_info.format = _format;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	_image = create_image(image_create_info);

	VulkanImageViewCreateInfo image_view_create_info{};
	image_view_create_info.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_create_info.layer_count = image_create_info.array_layers;
	image_view_create_info.mip_level_count = image_create_info.mip_levels;
	image_view_create_info.type = VK_IMAGE_VIEW_TYPE_2D;
	_image_view = create_image_view(_image, _format, image_view_create_info);

	StagingBuffer::copy_data_to_buffer(pixels, width * height * 4);

	VkImageSubresourceLayers subresource_layers;
	subresource_layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource_layers.baseArrayLayer = 0;
	subresource_layers.layerCount = 1;
	subresource_layers.mipLevel = 0;

	VkImageSubresourceRange subresource_range;
	subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource_range.baseArrayLayer = 0;
	subresource_range.baseMipLevel = 0;
	subresource_range.layerCount = 1;
	subresource_range.levelCount = 1;

	auto cmd = CommandManager::begin_single_command_buffer();

	CommandManager::transition_image_layout(cmd, *this,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource_range);

	StagingBuffer::copy_buffer_to_image(cmd, *this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource_layers);

	CommandManager::transition_image_layout(cmd, *this,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresource_range);

	CommandManager::end_single_command_buffer(cmd);

	stbi_image_free(pixels);

	create_sampler();
}

void VulkanTextureBase::create_sampler() {
	VkSamplerCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	create_info.addressModeV = create_info.addressModeU;
	create_info.addressModeW = create_info.addressModeU;
	create_info.anisotropyEnable = VK_FALSE;
	create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	create_info.magFilter = VK_FILTER_LINEAR;
	create_info.minFilter = VK_FILTER_LINEAR;
	create_info.minLod = 0.f;
	create_info.maxLod = 0.f;
	create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	create_info.unnormalizedCoordinates = VK_FALSE;
	create_info.compareEnable = VK_FALSE;
	VK_ASSERT(vkCreateSampler(Core::get_device(), &create_info, nullptr, &_sampler), "vkCreateSampler() - FAILED");
}