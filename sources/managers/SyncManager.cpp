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

	VkEventCreateInfo event_create_info{};
	event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

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

CurrentFrameSync SyncManager::get_current_frame_sync_objects() noexcept {
	uint32_t current_frame = Core::get_current_frame();

	CurrentFrameSync sync;
	sync.fence = _fences[current_frame];
	sync.semaphore_to_render = _semaphores_to_render[current_frame];

	sync.present_image_semaphores = { _semaphores_to_present[current_frame] };
	sync.signal_submit_semaphores = { _semaphores_to_present[current_frame] };

	sync.wait_semaphores = { sync.semaphore_to_render };
	sync.wait_submit_flags = { VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT };

	return sync;
}

SyncManager::~SyncManager() {
	VkDevice device = Core::get_device();
	for (uint32_t i = 0; i < _fences.size(); i++) {
		vkDestroyFence(device, _fences[i], nullptr);
		vkDestroySemaphore(device, _semaphores_to_present[i], nullptr);
		vkDestroySemaphore(device, _semaphores_to_render[i], nullptr);
	}
}