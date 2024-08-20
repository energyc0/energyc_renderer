#pragma once

#include "SceneObject.h"

class Scene {
private:
	std::vector<SceneObject*> _objects;
	VkDescriptorSetLayout _descriptor_set_layout;
	const std::unique_ptr<class MaterialManager>& _material_manager;


	class ModelGroup {
	private:
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

	private:
		void create_descriptor_tools(VkDescriptorSetLayout layout, const VulkanBuffer& scene_lights_buffer);
		void create_buffers(Mesh* object);
		void push_model(const Mesh* mesh);

	public:
		ModelGroup(Mesh* object, VkDescriptorSetLayout layout, const VulkanBuffer& scene_lights_buffer) noexcept;

		bool try_add_mesh(const Mesh* object);
		void draw(VkCommandBuffer command_buffer, VkPipelineLayout layout, const std::unique_ptr<class MaterialManager>& material_manager);

		~ModelGroup();
	};


	VulkanBuffer _scene_lights_buffer;
	std::vector<PointLight> _point_lights;
	std::vector<ModelGroup*> _object_groups;

private:
	void create_descriptor_tools();
	bool add_mesh(Mesh* mesh);
	bool add_point_light(PointLight* light);

public:
	Scene(const std::unique_ptr<class MaterialManager>& _material_manager, const std::vector<SceneObject*>& objects = {}) noexcept;

	inline VkDescriptorSetLayout get_descriptor_set_layout()const noexcept { return _descriptor_set_layout; }

	bool add_object(SceneObject* object);
	void draw(VkCommandBuffer command_buffer, VkPipelineLayout layout) const noexcept;

	~Scene();
};