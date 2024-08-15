#pragma once

#include "SceneObject.h"

class Scene {
private:
	std::vector<SceneObject*> _objects;

	class ModelGroup {
	private:
		std::vector<Model*> _render_objects;
		VulkanBuffer* _vertex_buffer;
		VulkanBuffer* _index_buffer;
	public:
		ModelGroup(Model* object);

		bool try_add_model(Model* object);
		void draw(VkCommandBuffer command_buffer);

		~ModelGroup();
	};

	std::vector<ModelGroup*> _object_groups;

public:
	Scene(const std::vector<SceneObject*>& objects);


	void add_object(SceneObject* object);
	void draw(VkCommandBuffer command_buffer);

	~Scene();
};