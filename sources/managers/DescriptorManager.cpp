#include "DescriptorManager.h"
#include "SceneObject.h"

constexpr uint32_t DESCRIPTOR_GROUP_MAX_SETS = 10;

//DescriptorManager::DescriptorGroup::DescriptorGroup(VkDescriptorSetLayout layout, const std::vector<VkDescriptorPoolSize>& pool_sizes) : _set_layout(layout){
//	VkDescriptorPoolCreateInfo create_info{};
//	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//	create_info.pPoolSizes = pool_sizes.data();
//	create_info.poolSizeCount = pool_sizes.size();
//	create_info.maxSets = DESCRIPTOR_GROUP_MAX_SETS;
//	VK_ASSERT(vkCreateDescriptorPool(Core::get_device(), &create_info, nullptr, &_pool), "vkCreateDescriptorPool(), RendererSolid - FAILED");
//	LOG_STATUS("Created DescriptorGroup.");
//}
//
//VkDescriptorSet DescriptorManager::DescriptorGroup::allocate_descriptor_set() const noexcept {
//	VkDescriptorSet set;
//
//	VkDescriptorSetAllocateInfo alloc_info{};
//	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//	alloc_info.descriptorPool = _pool;
//	alloc_info.descriptorSetCount = 1;
//	alloc_info.pSetLayouts = &_set_layout;
//	VK_ASSERT(vkAllocateDescriptorSets(Core::get_device(), &alloc_info, &set), "vkAllocateDescriptorSets(), RenderUnitSolid - FAILED.");
//
//	return set;
//}
//
//bool DescriptorManager::DescriptorGroup::operator==(const DescriptorGroup* group) const noexcept {
//	return this->_pool == group->_pool && this->_set_layout == group->_set_layout;
//}
//
//DescriptorManager::DescriptorGroup::~DescriptorGroup() {
//	vkDestroyDescriptorPool(Core::get_device(), _pool, nullptr);
//}
//
//DescriptorManager::DescriptorManager() {
//	VkDescriptorSetLayout _model_descriptor_set_layout;;
//
//	auto bindings = RenderUnitSolid::get_bindings();
//	VkDescriptorSetLayoutCreateInfo create_info{};
//	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//	create_info.bindingCount = bindings.size();
//	create_info.pBindings = bindings.data();
//	VK_ASSERT(vkCreateDescriptorSetLayout(Core::get_device(), &create_info, nullptr, &_descriptor_set_layout), "vkCreateDescriptorSetLayout(), RendererSolid - FAILED");
//
//	LOG_STATUS("Created DescriptorManager.");
//}
//
//VkDescriptorSet DescriptorManager::allocate_model_descriptor_set() noexcept {
//
//}
//
//DescriptorManager::~DescriptorManager() {
//	for (auto i : _groups) {
//		delete i;
//	}
//}