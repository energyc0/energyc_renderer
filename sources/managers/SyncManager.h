#pragma once
#include "Core.h"

class SyncManager {
private:
	std::vector<VkSemaphore> _semaphores_to_render;
	std::vector<VkSemaphore> _semaphores_to_present;
	std::vector<VkFence> _fences;


	static SyncManager* sync_manager_ptr;

private:
	void create_frame_sync_objects();

public:
	SyncManager();

	inline VkSemaphore get_semaphore_to_render() const noexcept { return _semaphores_to_render[Core::get_current_frame()]; }
	inline VkSemaphore get_semaphore_to_present() const noexcept { return _semaphores_to_present[Core::get_current_frame()]; }
	inline VkFence get_fence() const noexcept { return _fences[Core::get_current_frame()]; }

	~SyncManager();
};