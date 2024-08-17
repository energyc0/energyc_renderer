#pragma once

//#include "Core.h"
//
//class DescriptorManager {
//private:
//
//	class DescriptorGroup {
//	private:
//		VkDescriptorPool _pool;
//		VkDescriptorSetLayout _set_layout;
//
//	public:
//		DescriptorGroup(VkDescriptorSetLayout layout, const std::vector<VkDescriptorPoolSize>& pool_sizes);
//		
//		VkDescriptorSet allocate_descriptor_set() const noexcept;
//		bool operator==(const DescriptorGroup* group) const noexcept;
//
//		~DescriptorGroup();
//	};
//	VkDescriptorSetLayout _model_descriptor_set_layout;
//	std::vector<DescriptorGroup*> _groups;
//
//public:
//	DescriptorManager();
//
//	VkDescriptorSet allocate_model_descriptor_set() noexcept;
//
//	~DescriptorManager();
//};