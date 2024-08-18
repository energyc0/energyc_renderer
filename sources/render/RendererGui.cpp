#include "RendererGui.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "Window.h"

RendererGui::RendererGui(const Window& window, VkRenderPass render_pass, GuiInfo& gui_info) : _gui_info(gui_info){
	create_descriptor_tools();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Core::get_physical_device(), Core::get_surface(), &capabilities);

	ImGui_ImplGlfw_InitForVulkan(window.get_window(), false);

	ImGui_ImplVulkan_InitInfo init_info{};
	init_info.Instance = Core::get_instance();
	init_info.PhysicalDevice = Core::get_physical_device();
	init_info.Device = Core::get_device();
	init_info.DescriptorPool = _descriptor_pool;
	init_info.ImageCount = Core::get_swapchain_image_count();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Queue = Core::get_graphics_queue();
	init_info.QueueFamily = Core::get_graphics_queue_family_index();
	init_info.MinImageCount = capabilities.minImageCount;
	init_info.RenderPass = render_pass;
	init_info.Subpass = 0;

	ImGui_ImplVulkan_Init(&init_info);
	LOG_STATUS("Created RendererGui.");
}

void RendererGui::create_descriptor_tools() {
	VkDescriptorPoolSize pool_size{};
	pool_size.descriptorCount = 1;
	pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	VkDescriptorPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.pPoolSizes = &pool_size;
	create_info.poolSizeCount = 1;
	create_info.maxSets = 1;
	create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	VK_ASSERT(vkCreateDescriptorPool(Core::get_device(), &create_info, nullptr, &_descriptor_pool), "vkCreateDescriptorPool(), RenderUnitSolid - FAILED");
	LOG_STATUS("Created RendererUnitSolid descriptor pool.");
}

void RendererGui::fill_command_buffer(VkCommandBuffer command_buffer) {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	bool is_open = true;
	ImGui::Begin("Information", &is_open, 
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::Text("FPS: %f \nms: %f", 1000.0f/_gui_info.delta_time, _gui_info.delta_time);
	ImGui::SetWindowPos(ImVec2(0.f, 0.f));
	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
}

RendererGui::~RendererGui() {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}