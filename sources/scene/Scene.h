#pragma once

#include "SceneObject.h"

class Scene {
private:
	std::vector<SceneObject*> _objects;
	VkDescriptorSetLayout _descriptor_set_layout;
	const std::shared_ptr<class MaterialManager>& _material_manager;

	class ModelGroup {
	private:
		//for downloading a mesh
		VkFence _fence;

		std::vector<Model*> _models;
		VulkanBuffer* _vertex_buffer;
		VulkanBuffer* _index_buffer;

		std::vector<VulkanBuffer*> _storage_buffers;
		VkDescriptorPool _descriptor_pool;
		std::vector<VkDescriptorSet> _descriptor_sets;
		uint32_t _storage_buffer_space;

		uint32_t _total_vertices;
		uint32_t _total_indices;
		VkDeviceSize _empty_vertices;
		VkDeviceSize _empty_indices;
		std::vector<char*> _buffer_data_ptrs;

		Model* _last_pushed_model = nullptr;
	private:
		void create_descriptor_tools(VkDescriptorSetLayout layout, const std::vector<VulkanBuffer*>& scene_lights_buffers);
		void create_buffers(const std::shared_ptr<Mesh>& object);
		void push_model(const std::shared_ptr<Mesh>& mesh);

	public:
		ModelGroup(const std::shared_ptr<Mesh>& object, VkDescriptorSetLayout layout, const std::vector<VulkanBuffer*>& scene_lights_buffers) noexcept;

		bool try_add_mesh(const std::shared_ptr<Mesh>& object);
		void draw(VkCommandBuffer command_buffer, VkPipelineLayout layout, const std::shared_ptr<MaterialManager>& material_manager);

		inline Model* get_last_pushed_model()const noexcept { return _last_pushed_model; }
		inline std::vector<VkDescriptorSet> get_descriptor_sets() { return _descriptor_sets; }
		~ModelGroup();
	};


	std::vector<VulkanBuffer*> _scene_lights_buffers;
	std::vector<std::shared_ptr<PointLight>> _point_lights;
	std::vector<VkDescriptorSet> _point_light_descriptor_sets;
	std::vector<ModelGroup*> _object_groups;

private:
	void create_buffers();
	void create_descriptor_tools();

	void copy_light_new_info(const std::shared_ptr<PointLight>& light, uint32_t idx, VkCommandBuffer command_buffer) noexcept;
public:
	Scene(const std::shared_ptr<class MaterialManager>& _material_manager) noexcept;

	inline VkDescriptorSetLayout get_descriptor_set_layout()const noexcept { return _descriptor_set_layout; }

	bool add_mesh(const std::shared_ptr<Mesh>& mesh);
	bool add_point_light(const std::shared_ptr<PointLight>& light);

	void display_scene_info_gui(bool* is_window_opened) const noexcept;

	void update_descriptor_sets(VkCommandBuffer command_buffer) noexcept;

	void draw_solid(VkCommandBuffer command_buffer, VkPipelineLayout layout) const noexcept;
	void draw_light(VkCommandBuffer command_buffer, VkPipelineLayout layout);

	~Scene();
};