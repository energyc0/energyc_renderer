#include "VulkanDataObjects.h"

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
