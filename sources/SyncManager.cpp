#include "SyncManager.h"

SyncManager* SyncManager::sync_manager_ptr = nullptr;

SyncManager::SyncManager() {
	assert(sync_manager_ptr == nullptr && "There can be only one SyncManager.");
	sync_manager_ptr = this;
	create_frame_sync_objects();
}

void SyncManager::create_frame_sync_objects() {
	uint32_t image_count = Core::get_swapchain_image_count();
	_semaphores_to_present.resize(image_count);
	_semaphores_to_render.resize(image_count);
	_fences.resize(image_count);

	VkSemaphoreCreateInfo semaphore_create_info{};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fence_create_info{};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkDevice device = Core::get_device();
	for (uint32_t i = 0; i < image_count; i++) {
		VK_ASSERT((vkCreateSemaphore(device, &semaphore_create_info, nullptr, &_semaphores_to_render[i]) ||
		vkCreateSemaphore(device, &semaphore_create_info, nullptr, &_semaphores_to_present[i]) ||
		vkCreateFence(device,&fence_create_info, nullptr, &_fences[i])),
			"create_frame_sync_objects() - FAILED");
	}
	LOG_STATUS("Created synchronization objects.");
}

SyncManager::~SyncManager() {
	VkDevice device = Core::get_device();
	for (uint32_t i = 0; i < _fences.size(); i++) {
		vkDestroyFence(device, _fences[i], nullptr);
		vkDestroySemaphore(device, _semaphores_to_present[i], nullptr);
		vkDestroySemaphore(device, _semaphores_to_render[i], nullptr);
	}
}