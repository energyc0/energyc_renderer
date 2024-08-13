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

	uint32_t _present_queue_family_index = 0;
	VkQueue _present_queue = VK_NULL_HANDLE;
	uint32_t _graphics_queue_family_index = 0;
	VkQueue _graphics_queue = VK_NULL_HANDLE;

	struct SwapchainInfo {
		uint32_t width;
		uint32_t height;
		VkFormat format;
		uint32_t image_count;
		uint32_t image_index;
	};
	SwapchainInfo _swapchain_info;

	static Core* core_ptr;
public:
	Core(struct GLFWwindow* window, const char* application_name, const char* engine_name);

	static inline VkInstance get_instance() noexcept { return core_ptr->_instance; }
	static inline VkPhysicalDevice get_physical_device() noexcept { return core_ptr->_physical_device; }
	static inline VkDevice get_device() noexcept { return core_ptr->_device; }
	static inline VkSwapchainKHR get_swapchain() noexcept { return core_ptr->_swapchain; }

	static inline uint32_t get_swapchain_width() noexcept { return core_ptr->_swapchain_info.width; }
	static inline uint32_t get_swapchain_height() noexcept { return core_ptr->_swapchain_info.height; }
	static inline VkFormat get_swapchain_format() noexcept { return core_ptr->_swapchain_info.format; }
	static inline uint32_t get_swapchain_image_count() noexcept { return core_ptr->_swapchain_info.image_count; }
	static inline uint32_t get_image_index() noexcept { return core_ptr->_swapchain_info.image_index; }

	static std::vector<VkImage> get_swapchain_images() noexcept;
	~Core();
private:
	void create_instance(class GLFWwindow* window, const char* application_name, const char* engine_name, std::vector<const char*> available_layers);
	void pick_physical_device();
	void create_device(std::vector<const char*> available_layers);
	void create_swapchain(GLFWwindow* window);
};