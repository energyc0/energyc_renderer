#include "RenderManager.h"
#include "RenderUnitSolid.h"
#include "Camera.h"
#include "Scene.h"
#include "MaterialManager.h"
#include <array>

struct GlobalData {
	glm::mat4 view;
	glm::mat4 perspective;
};


RenderManager::RenderManager(const RenderManagerCreateInfo& render_manager_create_info) noexcept: 
	_camera(render_manager_create_info.camera),
	_scene(render_manager_create_info.scene),
	_material_manager(render_manager_create_info.material_manager){
	std::shared_ptr<VulkanImage> depth_image;
	std::shared_ptr<VulkanImageView> depth_image_view;
	create_images(depth_image, depth_image_view);
	create_buffers();
	create_descritor_tools();

	RenderUnitSolidCreateInfo solid_create_info{
		render_manager_create_info.scene,
		render_manager_create_info.material_manager,
		render_manager_create_info.camera,
		render_manager_create_info.window,
		render_manager_create_info.gui_info,
		depth_image,
		depth_image_view,
		_descriptor_set_layout
	};

	_render_units = { new RenderUnitSolid(solid_create_info) };
}

void RenderManager::create_images(std::shared_ptr<VulkanImage>& depth_image, std::shared_ptr<VulkanImageView>& depth_image_view) {

	VulkanImageCreateInfo image_create_info{};
	image_create_info.array_layers = 1;
	image_create_info.mip_levels = 1;
	image_create_info.format = Core::find_appropriate_format({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_IMAGE_TILING_OPTIMAL);
	image_create_info.width = Core::get_swapchain_width();
	image_create_info.height = Core::get_swapchain_height();
	image_create_info.memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depth_image = std::shared_ptr<VulkanImage>(new VulkanImage(image_create_info));

	VulkanImageViewCreateInfo view_create_info{};
	view_create_info.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_create_info.layer_count = 1;
	view_create_info.mip_level_count = 1;
	view_create_info.type = VK_IMAGE_VIEW_TYPE_2D;
	depth_image_view = std::shared_ptr<VulkanImageView>(new VulkanImageView(*depth_image.get(), view_create_info));

	LOG_STATUS("Created depth image and image view.");
}

void RenderManager::create_buffers() {
	auto image_count = Core::get_swapchain_image_count();
	_global_uniform_buffers.reserve(image_count);
	_global_uniform_memory_ptrs.resize(image_count);
	for (uint32_t i = 0; i < image_count; i++) {
		_global_uniform_buffers.push_back(new VulkanBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			sizeof(glm::mat4) * 2,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

		_global_uniform_memory_ptrs[i] = _global_uniform_buffers[i]->map_memory(0, VK_WHOLE_SIZE);
	}
	LOG_STATUS("Created RendererManager uniform buffers.");
}

void RenderManager::create_descritor_tools() {
	{
		std::array<VkDescriptorPoolSize, 1> pool_sizes{};
		pool_sizes[0].descriptorCount = Core::get_swapchain_image_count();
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkDescriptorPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.pPoolSizes = pool_sizes.data();
		create_info.poolSizeCount = pool_sizes.size();
		create_info.maxSets = Core::get_swapchain_image_count();
		VK_ASSERT(vkCreateDescriptorPool(Core::get_device(), &create_info, nullptr, &_descriptor_pool), "vkCreateDescriptorPool(), RenderUnitSolid - FAILED");
		LOG_STATUS("Created RenderUnitSolid descriptor pool.");
	}

	{
		auto bindings = get_bindings();
		VkDescriptorSetLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = bindings.size();
		create_info.pBindings = bindings.data();
		VK_ASSERT(vkCreateDescriptorSetLayout(Core::get_device(), &create_info, nullptr, &_descriptor_set_layout), "vkCreateDescriptorSetLayout(), RenderUnitSolid - FAILED");
		LOG_STATUS("Created RenderUnitSolid descriptor set layout.");
	}

	{
		_descriptor_sets.resize(Core::get_swapchain_image_count());
		std::vector<VkDescriptorSetLayout> layouts(_descriptor_sets.size(), _descriptor_set_layout);
		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = _descriptor_pool;
		alloc_info.descriptorSetCount = _descriptor_sets.size();
		alloc_info.pSetLayouts = layouts.data();
		VK_ASSERT(vkAllocateDescriptorSets(Core::get_device(), &alloc_info, _descriptor_sets.data()), "vkAllocateDescriptorSets(), RenderUnitSolid - FAILED.");
		LOG_STATUS("Allocated RenderUnitSolid descriptor sets.");
	}

	{
		std::vector<VkWriteDescriptorSet> descriptor_write;
		descriptor_write.reserve(_descriptor_sets.size());

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.dstArrayElement = 0;
		write.dstBinding = 0;
		for (uint32_t i = 0; i < _descriptor_sets.size(); i++) {
			VkDescriptorBufferInfo buffer_info = _global_uniform_buffers[i]->get_info(0, VK_WHOLE_SIZE);

			write.dstSet = _descriptor_sets[i];
			write.pBufferInfo = &buffer_info;
			descriptor_write.push_back(write);
		}
		vkUpdateDescriptorSets(Core::get_device(), descriptor_write.size(), descriptor_write.data(), 0, 0);
	}
}

void RenderManager::update_descriptor_sets(VkCommandBuffer command_buffer) {
	float aspect = static_cast<float>(Core::get_swapchain_width()) / static_cast<float>(Core::get_swapchain_height());

	GlobalData data;
	data.view = _camera.get_view_matrix();
	data.perspective = glm::perspective(glm::radians(90.f), aspect, 0.01f, 1000.f);
	//vulkan -y
	data.perspective[1][1] *= -1;

	memcpy(_global_uniform_memory_ptrs[Core::get_current_frame()], &data, sizeof(GlobalData));

	_scene->update_descriptor_sets(command_buffer);
	_material_manager->update_uniform_buffer(command_buffer);
}

std::vector<VkDescriptorSetLayoutBinding> RenderManager::get_bindings() noexcept {
	std::vector< VkDescriptorSetLayoutBinding> bindings(1);

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	return bindings;
}

void RenderManager::render(VkCommandBuffer command_buffer) {
	CurrentFrameData frame_data{
		_descriptor_sets[Core::get_current_frame()]
	};

	for (auto render_unit : _render_units) {
		render_unit->fill_command_buffer(command_buffer, frame_data);
	}
}

RenderManager::~RenderManager() {
	vkDestroyDescriptorPool(Core::get_device(), _descriptor_pool, nullptr);
	vkDestroyDescriptorSetLayout(Core::get_device(),_descriptor_set_layout, nullptr);
	for (auto ubo : _global_uniform_buffers) {
		delete ubo;
	}
	for (auto render_unit : _render_units) {
		delete render_unit;
	}
}