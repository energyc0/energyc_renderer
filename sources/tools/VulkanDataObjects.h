#pragma once

#include "Core.h"

class VulkanDataObject {
protected:
	static int32_t find_memory_type(uint32_t memory_type_bits, VkMemoryPropertyFlags memory_property) noexcept;
public:
	VulkanDataObject() noexcept {}
	VulkanDataObject(VulkanDataObject&& obj) noexcept {}
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
	VulkanResizable(uint32_t width, uint32_t height) noexcept  : _width(width), _height(height) {}
	VulkanResizable(VulkanResizable&& obj) noexcept : _width(obj._width), _height(obj._height) {}
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

	VulkanFramebufferBase(uint32_t width, uint32_t height) noexcept :
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
		VkRenderPass render_pass) noexcept;

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
		VkRenderPass render_pass) noexcept;

	~VulkanFramebuffer();
};

class VulkanBuffer : VulkanDataObject{
protected:
	VkBuffer _buffer;
	VkDeviceMemory _memory;
	VkDeviceSize _size;
	char* _data_ptr = nullptr;

protected:
	void create_vulkan_buffer(VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memory_property) noexcept;
	void free_vulkan_buffer() noexcept;
public:
	explicit VulkanBuffer(VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memory_property) noexcept;

	inline VkDeviceSize get_size()const noexcept { return _size; }
	inline char* map_memory(VkDeviceSize offset, VkDeviceSize size) noexcept {
		vkMapMemory(Core::get_device(), _memory, offset, size, NULL, (void**)&_data_ptr);
		return _data_ptr;
	}
	inline void unmap_memory() noexcept {
		vkUnmapMemory(Core::get_device(), _memory);
		_data_ptr = nullptr;
	}
	inline void bind_index_buffer(VkCommandBuffer command_buffer, VkDeviceSize offset) const noexcept {
		vkCmdBindIndexBuffer(command_buffer, _buffer, offset, VK_INDEX_TYPE_UINT32);
	}
	inline void bind_vertex_buffer(VkCommandBuffer command_buffer, VkDeviceSize offset) const noexcept {
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &_buffer, &offset);
	}

	VkDescriptorBufferInfo get_info(VkDeviceSize offset, VkDeviceSize range) const noexcept;

	void recreate(VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags memory_property) noexcept;

	~VulkanBuffer();

	friend class CommandManager;
	friend class StagingBuffer;
};

class StagingBuffer : private VulkanBuffer{
private:
	static VkDeviceSize _last_copied_size;
	static StagingBuffer* _buffer_ptr;
private:
	static void recreate_buffer() noexcept;

	static void copy_data_to_buffer(const void* data, size_t size) noexcept;
public:
	StagingBuffer();

	static void copy_buffers(VkCommandBuffer command_buffer, const void* data, size_t size, const class VulkanBuffer& dst,
		VkDeviceSize src_offset, VkDeviceSize dst_offset) noexcept;

	static void copy_buffer_to_image(VkCommandBuffer command_buffer, const void* data, size_t size, const class VulkanImage& dst_image,
		VkImageLayout dst_image_layout, const VkImageSubresourceLayers& subresource) noexcept;
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
protected:
	VkImage _image;
	VkDeviceMemory _memory;
	VkFormat _format;

protected:
	VulkanImage(VkFormat format) noexcept;

	VkImage create_image(const VulkanImageCreateInfo& image_create_info);
public:
	explicit VulkanImage(const VulkanImageCreateInfo& image_create_info) noexcept;
	VulkanImage(VulkanImage&& image) noexcept;

	inline VkImage get_image() const noexcept { return _image; }
	inline VkFormat get_format() const noexcept { return _format; }

	virtual ~VulkanImage();
};

struct VulkanTextureCreateInfo {
	VkFormat format;
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
protected:
	VkImageView _image_view;

protected:
	VulkanImageView() noexcept;
public:
	explicit VulkanImageView(VkImage image, VkFormat format, const VulkanImageViewCreateInfo& view_create_info) noexcept;

	explicit VulkanImageView(const VulkanImage& vulkan_image, const VulkanImageViewCreateInfo& view_create_info) noexcept;
	VulkanImageView(VulkanImageView&& image_view) noexcept;

	inline VkImageView get_image_view() const noexcept { return _image_view; }

	virtual ~VulkanImageView();
};

class VulkanMultipleImageViews : public VulkanImageViewBase {
private:
	std::vector<VkImageView> _image_views;

public:
	explicit VulkanMultipleImageViews(const std::vector<VkImage>& images, VkFormat format, const VulkanImageViewCreateInfo& view_create_info) noexcept;

	explicit VulkanMultipleImageViews(const std::vector<VulkanImage>& vulkan_images, const VulkanImageViewCreateInfo& view_create_info) noexcept;
	 
	VulkanMultipleImageViews(VulkanMultipleImageViews&& image_views) noexcept;

	~VulkanMultipleImageViews();

	inline VkImageView get(size_t idx) const noexcept { return _image_views[idx]; }
	inline const std::vector<VkImageView>& get_image_views() const noexcept { return _image_views; }
};

class VulkanTextureBase : public VulkanImage, public VulkanImageView {
protected:
	VkSampler _sampler;

protected:
	VulkanTextureBase(VkFormat format) noexcept;

	VulkanTextureBase(VulkanTextureBase&& texture) noexcept;
	VulkanTextureBase(const VulkanTextureBase& texture) noexcept;

	void create_sampler();
public:
	virtual ~VulkanTextureBase();
};

class VulkanTexture2D : public VulkanTextureBase{
private:
	void load_texture(const char* filename);

public:
	VulkanTexture2D();
	VulkanTexture2D(VkFormat format, const char* filename) noexcept;
	VulkanTexture2D(VulkanTexture2D&& texture) noexcept;

	VkDescriptorImageInfo get_info(VkImageLayout layout) const noexcept;
};
