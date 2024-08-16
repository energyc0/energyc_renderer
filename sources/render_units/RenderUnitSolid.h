#pragma once

#include "RenderUnitBase.h"
#include "RendererSolid.h"
#include "Camera.h"

class RenderUnitSolid : public RenderUnitBase{
private:
	RendererSolid* _renderer_solid;
	VulkanMultipleImageViews* _image_views;
	const CameraBase& _camera;


	std::vector<VulkanBuffer*> _uniform_buffers;
	std::vector<char*> _uniform_memory_ptrs;
	std::vector<VkDescriptorSet> _descriptor_sets;
	VkDescriptorPool _descriptor_pool;
	VkDescriptorSetLayout _descriptor_set_layout;
	VkPipelineLayout _pipeline_layout;

private:
	virtual void create_render_pass();
	virtual void create_framebuffers();
	void create_descriptor_tools();
	void create_buffers();

public:
	RenderUnitSolid(class Scene& scene, const CameraBase& camera);

	virtual void fill_command_buffer(VkCommandBuffer command_buffer);
	void update_descriptor_sets();

	static std::vector<VkDescriptorSetLayoutBinding> get_bindings() noexcept;

	~RenderUnitSolid();
};