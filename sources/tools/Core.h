#pragma once

#include "Utils.h"

class Core {
private:
	VkInstance _instance = VK_NULL_HANDLE;
	VkSurfaceKHR _surface = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT _debug_messenger = VK_NULL_HANDLE;
	VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

	uint32_t _present_queue_family_index = UINT32_MAX;
	VkQueue _present_queue = VK_NULL_HANDLE;
	uint32_t _graphics_queue_family_index = UINT32_MAX;
	VkQueue _graphics_queue = VK_NULL_HANDLE;

	struct SwapchainInfo {
		uint32_t width;
		uint32_t height;
		VkFormat format;
		uint32_t image_count;
		uint32_t image_index = 0;
		uint32_t current_frame = 1;
		uint32_t previous_frame = 0;
	};
	SwapchainInfo _swapchain_info;

	VkDeviceSize _min_uniform_offset_alignment;

	static Core* core_ptr;
public:
	Core(struct GLFWwindow* window, const char* application_name, const char* engine_name);

	static inline VkInstance get_instance() noexcept { return core_ptr->_instance; }
	static inline VkPhysicalDevice get_physical_device() noexcept { return core_ptr->_physical_device; }
	static inline VkSurfaceKHR get_surface() noexcept { return core_ptr->_surface; }
	static inline VkDevice get_device() noexcept { return core_ptr->_device; }
	static inline VkSwapchainKHR get_swapchain() noexcept { return core_ptr->_swapchain; }

	static inline VkQueue get_graphics_queue() noexcept { return core_ptr->_graphics_queue; }
	static inline VkQueue get_present_queue() noexcept { return core_ptr->_present_queue; }
	static inline uint32_t get_graphics_queue_family_index() noexcept { return core_ptr->_graphics_queue_family_index; }
	static inline uint32_t get_present_queue_family_index() noexcept { return core_ptr->_present_queue_family_index; }

	static inline uint32_t get_swapchain_width() noexcept { return core_ptr->_swapchain_info.width; }
	static inline uint32_t get_swapchain_height() noexcept { return core_ptr->_swapchain_info.height; }
	static inline VkFormat get_swapchain_format() noexcept { return core_ptr->_swapchain_info.format; }
	static inline uint32_t get_swapchain_image_count() noexcept { return core_ptr->_swapchain_info.image_count; }
	static inline uint32_t get_image_index() noexcept { return core_ptr->_swapchain_info.image_index; }
	static inline uint32_t get_current_frame() noexcept { return core_ptr->_swapchain_info.current_frame; }
	static inline uint32_t get_previous_frame() noexcept { return core_ptr->_swapchain_info.previous_frame; }

	static inline VkDeviceSize get_min_uniform_offset_alignment() noexcept { return core_ptr->_min_uniform_offset_alignment; }

	static VkFormat find_appropriate_format(const std::vector<VkFormat>& candidates, VkFormatFeatureFlagBits features, VkImageTiling tiling) noexcept;

	inline VkResult acquire_next_image(VkSemaphore semaphore, VkFence fence) noexcept{
		return vkAcquireNextImageKHR(_device, _swapchain, UINT32_MAX, semaphore, fence, &_swapchain_info.image_index);
	}
	inline VkResult queue_present(const std::vector<VkSemaphore>& wait_semaphores) {
		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pImageIndices = &_swapchain_info.image_index;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &_swapchain;
		present_info.waitSemaphoreCount = wait_semaphores.size();
		present_info.pWaitSemaphores = wait_semaphores.data();
		return vkQueuePresentKHR(Core::get_graphics_queue(), &present_info);
	}

	inline void next_frame() noexcept {
		_swapchain_info.previous_frame = _swapchain_info.current_frame;
		_swapchain_info.current_frame = (_swapchain_info.current_frame + 1) % _swapchain_info.image_count;
	};


	static std::vector<VkImage> get_swapchain_images() noexcept;
	~Core();
private:
	void create_instance(class GLFWwindow* window, const char* application_name, const char* engine_name, std::vector<const char*> available_layers);
	void pick_physical_device();
	void create_device(std::vector<const char*> available_layers);
	void create_swapchain(GLFWwindow* window);
};