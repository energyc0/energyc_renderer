#include "RenderUnitSolid.h"
#include "Camera.h"
#include <array>

RenderUnitSolid::RenderUnitSolid(class Scene& scene, const CameraBase& camera) : _camera(camera) {
	create_render_pass();
	create_framebuffers();
	create_buffers();
	create_descriptor_tools();

	RendererSolidCreateInfo create_info{
		_render_pass,
		_descriptor_set_layout,
		_descriptor_sets,
		scene
	};
	_renderer_solid = new RendererSolid(&create_info);
	LOG_STATUS("RenderUnitSolid - RendererSolid created.");
}

RenderUnitSolid::~RenderUnitSolid() {
	uint32_t image_count = Core::get_swapchain_image_count();
	for (uint32_t i = 0; i < image_count;i++) {
		delete _uniform_buffers[i];
	}
	vkDestroyPipelineLayout(Core::get_device(), _pipeline_layout, nullptr);
	vkDestroyDescriptorPool(Core::get_device(), _descriptor_pool, nullptr);
	vkDestroyDescriptorSetLayout(Core::get_device(), _descriptor_set_layout, nullptr);
	delete _image_views;
	delete _renderer_solid;
}

void RenderUnitSolid::create_render_pass() {
	VkAttachmentDescription color_attachment{};
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	color_attachment.format = Core::get_swapchain_format();
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference color_attachment_reference{};
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_reference;

	std::array<VkSubpassDependency,1> dependency{};
	dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency[0].dstSubpass = 0;
	dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[0].dependencyFlags = 0;


	VkRenderPassCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	create_info.pSubpasses = &subpass;
	create_info.subpassCount = 1;
	create_info.attachmentCount = 1;
	create_info.pAttachments = &color_attachment;
	create_info.pDependencies = dependency.data();
	create_info.dependencyCount = dependency.size();

	VK_ASSERT(vkCreateRenderPass(Core::get_device(), &create_info, nullptr, &_render_pass), "vkCreateRenderPass(), RenderUnitSolid - FAILED");
	LOG_STATUS("RenderUnitSolid - render pass created.");
}

void RenderUnitSolid::create_framebuffers() {
	auto images = Core::get_swapchain_images();

	_image_views = new VulkanMultipleImageViews(images, Core::get_swapchain_format(), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
	LOG_STATUS("RenderUnitSolid - image views created.");
	_framebuffer = new VulkanMultipleFramebuffers(Core::get_swapchain_width(), Core::get_swapchain_height(), _image_views->get_image_views(), _render_pass, images.size());
	LOG_STATUS("RenderUnitSolid - framebuffers created.");
}

void RenderUnitSolid::create_buffers() {
	auto image_count = Core::get_swapchain_image_count();
	_uniform_buffers.reserve(image_count);
	_uniform_memory_ptrs.resize(image_count);
	for (uint32_t i = 0; i < image_count; i++) {
		_uniform_buffers.push_back(new VulkanBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			sizeof(glm::mat4) * 2,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

		_uniform_memory_ptrs[i] = _uniform_buffers[i]->map_memory(0, VK_WHOLE_SIZE);
	}
}

void RenderUnitSolid::create_descriptor_tools() {
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
		LOG_STATUS("Created RendererSolid descriptor pool.");
	}

	{
		auto bindings = get_bindings();
		VkDescriptorSetLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = bindings.size();
		create_info.pBindings = bindings.data();
		VK_ASSERT(vkCreateDescriptorSetLayout(Core::get_device(), &create_info, nullptr, &_descriptor_set_layout), "vkCreateDescriptorSetLayout(), RenderUnitSolid - FAILED");
	}

	{
		_descriptor_sets.resize(Core::get_swapchain_image_count());
		std::vector<VkDescriptorSetLayout> layouts(_descriptor_sets.size(), _descriptor_set_layout);
		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = _descriptor_pool;
		alloc_info.descriptorSetCount = _descriptor_sets.size();
		alloc_info.pSetLayouts = layouts.data();
		VK_ASSERT(vkAllocateDescriptorSets(Core::get_device(), &alloc_info, _descriptor_sets.data()),"vkAllocateDescriptorSets(), RenderUnitSolid - FAILED.");
	}

	{
		VkPipelineLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.setLayoutCount = 1;
		create_info.pSetLayouts = &_descriptor_set_layout;
		VK_ASSERT(vkCreatePipelineLayout(Core::get_device(), &create_info, nullptr, &_pipeline_layout), "vkCreatePipelineLayout(), RenderUnitSolid - FAILED.");
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
			VkDescriptorBufferInfo buffer_info = _uniform_buffers[i]->get_info(0,VK_WHOLE_SIZE);

			write.dstSet = _descriptor_sets[i];
			write.pBufferInfo = &buffer_info;
			descriptor_write.push_back(write);
		}
		vkUpdateDescriptorSets(Core::get_device(), descriptor_write.size(), descriptor_write.data(), 0, 0);
	}
}

void RenderUnitSolid::update_descriptor_sets() {
	float aspect = static_cast<float>(Core::get_swapchain_width()) / static_cast<float>(Core::get_swapchain_height());

	glm::mat4 matrices[2];
	matrices[0] = _camera.get_view_matrix();
	matrices[1] = glm::perspective(glm::radians(90.f), aspect, 0.01f, 1000.f);
	matrices[1][1][1] *= -1;

	memcpy(_uniform_memory_ptrs[Core::get_current_frame()], matrices, _uniform_buffers[Core::get_current_frame()]->get_size());
}

std::vector<VkDescriptorSetLayoutBinding> RenderUnitSolid::get_bindings() noexcept {
	std::vector< VkDescriptorSetLayoutBinding> bindings(1);

	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	return bindings;
}

void RenderUnitSolid::fill_command_buffer(VkCommandBuffer command_buffer) {
	VkClearValue clear_value{};
	clear_value.color = { 0.0f,0.0f,0.0f };
	VkRenderPassBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	begin_info.clearValueCount = 1;
	begin_info.pClearValues = &clear_value;
	begin_info.framebuffer = _framebuffer->get_framebuffer();
	begin_info.renderPass = _render_pass;
	begin_info.renderArea.offset = { 0,0 };
	begin_info.renderArea.extent = { _framebuffer->get_width(), _framebuffer->get_height() };

	vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
	
	VkViewport viewport{};
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = _framebuffer->get_width();
	viewport.height = _framebuffer->get_height();
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &begin_info.renderArea);

	_renderer_solid->fill_command_buffer(command_buffer);

	vkCmdEndRenderPass(command_buffer);
}