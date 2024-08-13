#include "Core.h"
#include "Window.h"
#include <iostream>
#include <array>
#include <unordered_set>
#include <optional>

constexpr uint32_t required_layers_count = 2;
constexpr std::array<const char*, required_layers_count> required_layers{ "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2" };
constexpr uint32_t required_device_extension_count = 1;
constexpr std::array<const char*, required_device_extension_count> required_device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

Core* Core::core_ptr = nullptr;

static VKAPI_ATTR VkBool32 VKAPI_CALL messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
							VkDebugUtilsMessageTypeFlagsEXT message_types,
							const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
							void* user_data) {
	std::cerr << "Validation layer: " << callback_data->pMessage << '\n';
	return VK_FALSE;
}



Core::Core(GLFWwindow* window, const char* application_name, const char* engine_name) {

	assert(core_ptr == nullptr && "There is only one core instance.");
	core_ptr = this;

	std::vector<const char*> available_layers;
	create_instance(window, application_name,engine_name, available_layers);
	pick_physical_device();
	create_device(available_layers);
	create_swapchain(window);
}

void Core::create_instance(GLFWwindow* window, const char* application_name, const char* engine_name, std::vector<const char*> available_layers) {
	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	uint32_t extensions_count = 0;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);

	std::vector<const char*> extensions(glfw_extensions, glfw_extensions + extensions_count);

#ifdef DEBUG

	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	uint32_t layers_count;
	vkEnumerateInstanceLayerProperties(&layers_count, nullptr);
	std::vector<VkLayerProperties> layer_properties(layers_count);
	vkEnumerateInstanceLayerProperties(&layers_count, layer_properties.data());

	available_layers.reserve(required_layers_count);

	for (auto& layer : required_layers) {
		bool is_found = false;
		for (auto& i : layer_properties) {
			if (strcmp(layer, i.layerName) == 0) {
				is_found = true;
				break;
			}
		}
		if (is_found) {
			available_layers.push_back(layer);
		}
		else {
			LOG_WARNING(layer, " layer is not found.");
		}
	}

	create_info.ppEnabledLayerNames = available_layers.data();
	create_info.enabledLayerCount = available_layers.size();

	VkDebugUtilsMessengerCreateInfoEXT messenger_create_info{};
	messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT|
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
	messenger_create_info.pfnUserCallback = static_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(messenger_callback);

	create_info.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&messenger_create_info);
#else

	create_info.ppEnabledLayerNames = nullptr;
	create_info.enabledLayerCount = 0;

#endif

	VkApplicationInfo application_info{};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pApplicationName = application_name;
	application_info.pEngineName = engine_name;
	application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.apiVersion = VK_API_VERSION_1_0;

	create_info.enabledExtensionCount = extensions.size();
	create_info.ppEnabledExtensionNames = extensions.data();
	create_info.pApplicationInfo = &application_info;
	VK_ASSERT(vkCreateInstance(&create_info, nullptr, &_instance), "vkCreateInstance() - FAILED.");
	LOG_STATUS("Instance created.");

#ifdef DEBUG
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		VK_ASSERT(func(_instance, &messenger_create_info, nullptr, &_debug_messenger), "vkCreateDebugUtilsMessengerEXT() - FAILED");
		LOG_STATUS("Debug utils messenger created.");
	}
	else {
		LOG_WARNING("vkCreateDebugUtilsMessengerEXT() - not found.");
	}
#endif // DEBUG



	VK_ASSERT(glfwCreateWindowSurface(_instance, window, nullptr, &_surface), "glfwCreateWindowSurface() - FAILED");
	LOG_STATUS("Surface created.");
}

void Core::pick_physical_device() {
	uint32_t phys_dev_count;
	vkEnumeratePhysicalDevices(_instance, &phys_dev_count, nullptr);
	std::vector<VkPhysicalDevice> phys_devices(phys_dev_count);
	vkEnumeratePhysicalDevices(_instance, &phys_dev_count, phys_devices.data());

	auto is_suitable_device = [this](VkPhysicalDevice phys_dev) {
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;

		uint32_t queue_family_count;
		vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &queue_family_count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &queue_family_count, queue_family_properties.data());

		uint32_t extension_properties_count;
		vkEnumerateDeviceExtensionProperties(phys_dev, nullptr, &extension_properties_count, nullptr);
		std::vector<VkExtensionProperties> extensions_properties(extension_properties_count);
		vkEnumerateDeviceExtensionProperties(phys_dev, nullptr, &extension_properties_count, extensions_properties.data());

		for (auto& i : required_device_extensions) {
			bool is_found = false;
			for (auto& extension : extensions_properties) {
				if (strcmp(i, extension.extensionName) == 0) {
					is_found = true;
					break;
				}
			}
			if (!is_found) {
				return false;
			}
		}

		for (uint32_t idx = 0; idx < queue_family_count; idx++) {
			if (queue_family_properties[idx].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				graphics_family = idx;
			}

			VkBool32 is_present_supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(phys_dev, idx, _surface, &is_present_supported);

			if (is_present_supported) {
				present_family = idx;
			}
		}
		bool result = graphics_family.has_value() && present_family.has_value();
		if (result) {
			_graphics_queue_family_index = graphics_family.value();
			_present_queue_family_index = present_family.value();
		}
		return result;
	};

	for (auto& phys_dev : phys_devices) {
		if (is_suitable_device(phys_dev)) {
			_physical_device = phys_dev;
			LOG_STATUS("Physical device is chosen.");
			break;
		}
	}
	if (_physical_device == VK_NULL_HANDLE) {
		LOG_ERROR("Compatible physical device is not found.");
	}

}

void Core::create_device(std::vector<const char*> available_layers) {

	VkDeviceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	
	constexpr float priority = 1.f;
	std::unordered_set<uint32_t> unique_queue_families{ _graphics_queue_family_index,_present_queue_family_index };
	std::vector<VkDeviceQueueCreateInfo> queue_create_info(unique_queue_families.size());
	
	if(queue_create_info.size() == 1){
		queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info[0].queueFamilyIndex = _graphics_queue_family_index;
		queue_create_info[0].pQueuePriorities = &priority;
		queue_create_info[0].queueCount = 1;
		LOG_STATUS("Graphics and present queue family indices are the same.");
	}
	else {
		queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info[0].queueFamilyIndex = _graphics_queue_family_index;
		queue_create_info[0].pQueuePriorities = &priority;
		queue_create_info[0].queueCount = 1;

		queue_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info[1].queueFamilyIndex = _present_queue_family_index;
		queue_create_info[1].pQueuePriorities = &priority;
		queue_create_info[1].queueCount = 1;
		LOG_STATUS("Graphics and present queue family indices are different.");
	}
	
	create_info.enabledExtensionCount = required_device_extension_count;
	create_info.ppEnabledExtensionNames = required_device_extensions.data();
	create_info.queueCreateInfoCount = queue_create_info.size();
	create_info.pQueueCreateInfos = queue_create_info.data();

	create_info.enabledLayerCount = available_layers.size();
	create_info.ppEnabledLayerNames = available_layers.data();
	VK_ASSERT(vkCreateDevice(_physical_device, &create_info, nullptr, &_device), "vkCreateDevice() - FAILED");

	LOG_STATUS("Created device.");

	vkGetDeviceQueue(_device, _graphics_queue_family_index, 0, &_graphics_queue);
	LOG_STATUS("Got graphics queue.");
	vkGetDeviceQueue(_device, _present_queue_family_index, 0, &_present_queue);
	LOG_STATUS("Got present queue.");
}

void Core::create_swapchain(GLFWwindow* window) {

	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physical_device, _surface, &surface_capabilities);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	uint32_t surface_formats_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physical_device, _surface, &surface_formats_count, nullptr);
	std::vector<VkSurfaceFormatKHR> surface_formats(surface_formats_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physical_device, _surface, &surface_formats_count, surface_formats.data());
	
	uint32_t present_modes_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(_physical_device, _surface, &present_modes_count, nullptr);
	std::vector<VkPresentModeKHR> present_modes(present_modes_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(_physical_device, _surface, &present_modes_count, present_modes.data());

	_swapchain_info.height = height;
	_swapchain_info.width = width;
	

	VkSurfaceFormatKHR preffered_format = surface_formats[0];
	int format_priority = INT_MAX;
	for (auto& i : surface_formats) {
		if (i.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
			if (i.format == VK_FORMAT_R8G8B8A8_SRGB) {
				preffered_format = i;
				format_priority = 0;
			}
			else if (i.format == VK_FORMAT_B8G8R8A8_SRGB && format_priority > 1) {
				preffered_format = i;
				format_priority = 1;
			}
			else if (i.format == VK_FORMAT_R8G8B8A8_UNORM && format_priority > 2) {
				preffered_format = i;
				format_priority = 2;

			}else if (i.format == VK_FORMAT_B8G8R8A8_UNORM && format_priority > 3) {
				preffered_format = i;
				format_priority = 3;

			}
		}
	}

	_swapchain_info.format = preffered_format.format;

	switch(preffered_format.format){
		case VK_FORMAT_R8G8B8A8_SRGB:LOG_STATUS("Format is chosen: VK_FORMAT_R8G8B8A8_SRGB"); break;
		case VK_FORMAT_B8G8R8A8_SRGB:LOG_STATUS("Format is chosen: VK_FORMAT_B8G8R8A8_SRGB"); break;
		case VK_FORMAT_R8G8B8A8_UNORM:LOG_STATUS("Format is chosen: VK_FORMAT_R8G8B8A8_UNORM"); break;
		case VK_FORMAT_B8G8R8A8_UNORM:LOG_STATUS("Format is chosen: VK_FORMAT_B8G8R8A8_UNORM"); break;
		default:
			LOG_WARNING("Failed to find the right format, choosing randomly.");
	}

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (auto& i : present_modes) {
		if (i == VK_PRESENT_MODE_MAILBOX_KHR) {
			present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}
	if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
		LOG_STATUS("Present mode is chosen: VK_PRESENT_MODE_MAILBOX_KHR");
	}
	else {
		LOG_STATUS("Present mode is chosen: VK_PRESENT_MODE_FIFO_KHR");
	}

	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = _surface;
	create_info.clipped = VK_TRUE;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.imageArrayLayers = 1;
	create_info.imageColorSpace = preffered_format.colorSpace;
	create_info.imageFormat = _swapchain_info.format;
	create_info.imageExtent = { _swapchain_info.width, _swapchain_info.height };
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.minImageCount = surface_capabilities.minImageCount + 1;
	if (create_info.minImageCount > surface_capabilities.maxImageCount) {
		create_info.minImageCount = surface_capabilities.minImageCount;
	}
	create_info.preTransform = surface_capabilities.currentTransform;
	create_info.presentMode = present_mode;

	if (_graphics_queue_family_index == _present_queue_family_index) {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;
	}
	else {
		uint32_t indices[] = { _graphics_queue_family_index, _present_queue_family_index };
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = indices;
	}


	VK_ASSERT(vkCreateSwapchainKHR(_device, &create_info, nullptr, &_swapchain), "vkCreateSwapchainKHR() - FAILED.");
	LOG_STATUS("Created swapchain.");


	vkGetSwapchainImagesKHR(_device, _swapchain, &_swapchain_info.image_count, nullptr);
	_swapchain_info.image_index = 0;
}

std::vector<VkImage> Core::get_swapchain_images() noexcept {
	uint32_t image_count = Core::get_swapchain_image_count();
	std::vector<VkImage> images(image_count);
	vkGetSwapchainImagesKHR(Core::get_device(), Core::get_swapchain(), &image_count, images.data());

	return images;
}

Core::~Core() {
	vkDestroySwapchainKHR(_device, _swapchain, nullptr);
	vkDestroyDevice(_device, nullptr);
	vkDestroySurfaceKHR(_instance, _surface, nullptr);

#ifdef DEBUG
	if (_debug_messenger != VK_NULL_HANDLE) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(_instance, _debug_messenger, nullptr);
		}
		else {
			LOG_WARNING("vkDestroyDebugUtilsMessengerEXT() - not found.");
		}
	}
#endif // DEBUG

	vkDestroyInstance(_instance, nullptr);
}