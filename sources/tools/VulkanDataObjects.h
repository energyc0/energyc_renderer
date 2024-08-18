#pragma once

#include "Core.h"

class VulkanDataObject {
protected:
	static int32_t find_memory_type(uint32_t memory_type_bits, VkMemoryPropertyFlags memory_property) noexcept;
public:
	VulkanDataObject() {}
	VulkanDataObject(const VulkanDataObject& obj) = delete;
	VulkanDataObject& operator=(VulkanDataObject& obj) = delete;
	virtual ~VulkanDataObject() {}
};

class VulkanResizable : public VulkanDataObject {
protected:
	uint32_t _width;
	uint32_t _height;

protected:
	virtual ~VulkanResizable() {}
	VulkanResizable(uint32_t width, uint32_t height) : _width(width), _height(height) {}
public:

	inline uint32_t get_width() const noexcept { return _width; }
	inline uint32_t get_height() const noexcept { return _height; }
};

class VulkanFramebufferBase : public VulkanResizable {
protected:
	static VkFramebuffer create_framebuffer(
		uint32_t width,
		uint32_t height,
		const std::vector<VkImageView>& attachments,
		VkRenderPass render_pass) noexcept;

	VulkanFramebufferBase(uint32_t width, uint32_t height) :
		VulkanResizable(width,height) {}

public:
	virtual ~VulkanFramebufferBase() {}

	virtual inline VkFramebuffer get_framebuffer() const noexcept = 0;
};

class VulkanMultipleFramebuffers : public VulkanFramebufferBase {
private:
	std::vector<VkFramebuffer> _framebuffers;

public:
	inline VkFramebuffer get_framebuffer() const noexcept { return _framebuffers[Core::get_image_index()]; }

	explicit VulkanMultipleFramebuffers(uint32_t width,
		uint32_t height,
		const std::vector<std::vector<VkImageView>>&,
		VkRenderPass render_pass);

	~VulkanMultipleFramebuffers();
};

class VulkanFramebuffer : public VulkanFramebufferBase {
private:
	VkFramebuffer _framebuffer;

public:
	inline VkFramebuffer get_framebuffer() const noexcept { return _framebuffer; }

	VulkanFramebuffer(uint32_t width,
		uint32_t height,
		std::vector<VkImageView>& attachments,
		VkRenderPass render_pass);

	~VulkanFramebuffer();
};

class VulkanBuffer : VulkanDataObject{
protected:
	VkBuffer _buffer;
	VkDeviceMemory _memory;
	VkDeviceSize _size;
	char* _ptr;

public:
	static void copy_buffers(VkCommandBuffer command_buffer,
		const VulkanBuffer& src, const VulkanBuffer& dst,
		VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size) noexcept;

	explicit VulkanBuffer(VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memory_property);

	inline VkDeviceSize get_size()const noexcept { return _size; }
	inline char* map_memory(VkDeviceSize offset, VkDeviceSize size) noexcept {
		vkMapMemory(Core::get_device(), _memory, offset, size, NULL, (void**)&_ptr);
		return _ptr;
	}
	inline void unmap_memory() noexcept {
		vkUnmapMemory(Core::get_device(), _memory);
		_ptr = nullptr;
	}
	inline void bind_index_buffer(VkCommandBuffer command_buffer, VkDeviceSize offset) const noexcept {
		vkCmdBindIndexBuffer(command_buffer, _buffer, offset, VK_INDEX_TYPE_UINT32);
	}
	inline void bind_vertex_buffer(VkCommandBuffer command_buffer, VkDeviceSize offset) const noexcept {
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &_buffer, &offset);
	}

	VkDescriptorBufferInfo get_info(VkDeviceSize offset, VkDeviceSize range) const noexcept;

	~VulkanBuffer();
};

struct VulkanImageCreateInfo {
	uint32_t width;
	uint32_t height;
	VkFormat format;
	uint32_t array_layers;
	uint32_t mip_levels;
	VkSampleCountFlagBits samples;
	VkImageUsageFlags usage;
	VkMemoryPropertyFlags memory_property;
};


class VulkanImage : public VulkanResizable {
private:
	VkImage _image;
	VkDeviceMemory _memory;
	VkFormat _format;

public:
	explicit VulkanImage(const VulkanImageCreateInfo& image_create_info);

	inline VkImage get_image() const noexcept { return _image; }
	inline VkFormat get_format() const noexcept { return _format; }

	~VulkanImage();
};

class VulkanTexture : public VulkanImage {

};

struct VulkanImageViewCreateInfo {
	VkImageViewType type;
	VkImageAspectFlags aspect;
	uint32_t layer_count;
	uint32_t mip_level_count;
};

class VulkanImageViewBase : public VulkanDataObject {
protected:
	static VkImageView create_image_view(
		VkImage image, VkFormat format, const VulkanImageViewCreateInfo& view_create_info);

	virtual ~VulkanImageViewBase() {}
};

class VulkanImageView : public VulkanImageViewBase {
private:
	VkImageView _image_view;

public:
	explicit VulkanImageView(VkImage image, VkFormat format, const VulkanImageViewCreateInfo& view_create_info);

	explicit VulkanImageView(const VulkanImage& vulkan_image, const VulkanImageViewCreateInfo& view_create_info);

	inline VkImageView get_image_view() const noexcept { return _image_view; }

	~VulkanImageView();
};

class VulkanMultipleImageViews : public VulkanImageViewBase {
private:
	std::vector<VkImageView> _image_views;

public:
	explicit VulkanMultipleImageViews(const std::vector<VkImage>& images, VkFormat format, const VulkanImageViewCreateInfo& view_create_info);

	explicit VulkanMultipleImageViews(const std::vector<VulkanImage>& vulkan_images, const VulkanImageViewCreateInfo& view_create_info);

	~VulkanMultipleImageViews();

	inline VkImageView get(size_t idx) const noexcept { return _image_views[idx]; }
	inline const std::vector<VkImageView>& get_image_views() const noexcept { return _image_views; }
};
