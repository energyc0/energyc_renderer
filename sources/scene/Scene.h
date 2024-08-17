#pragma once

#include "SceneObject.h"

class Scene {
private:
	std::vector<SceneObject*> _objects;
	VkDescriptorSetLayout _model_group_layout;


	class ModelGroup {
	private:
		std::vector<Model*> _models;
		VulkanBuffer* _vertex_buffer;
		VulkanBuffer* _index_buffer;

		std::vector<VulkanBuffer*> _storage_buffers;
		VkDescriptorPool _descriptor_pool;
		std::vector<VkDescriptorSet> _descriptor_sets;
		
		uint32_t _total_vertices;
		uint32_t _total_indices;
		std::vector<char*> _buffer_data_ptrs;

	private:
		void create_descriptor_tools(VkDescriptorSetLayout layout);
		void create_buffers(Mesh* object);
		void push_model(const Mesh* mesh);

	public:
		ModelGroup(Mesh* object, VkDescriptorSetLayout layout);

		bool try_add_mesh(const Mesh* object);
		void draw(VkCommandBuffer command_buffer, VkPipelineLayout layout, VkDescriptorSet global_ubo);

		~ModelGroup();
	};

	std::vector<ModelGroup*> _object_groups;

public:
	Scene(const std::vector<SceneObject*>& objects);

	inline VkDescriptorSetLayout get_descriptor_set_layout()const noexcept { return _model_group_layout; }

	void add_object(SceneObject* object);
	void draw(VkCommandBuffer command_buffer, VkPipelineLayout layout, VkDescriptorSet global_ubo);

	~Scene();
};