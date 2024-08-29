#pragma once
#include "Core.h"

struct CurrentFrameSync {
	VkFence fence;
	VkSemaphore semaphore_to_render;

	std::vector<VkSemaphore> present_image_semaphores;
	std::vector<VkSemaphore> signal_submit_semaphores;
	std::vector<VkSemaphore> wait_semaphores;
	std::vector<VkPipelineStageFlags> wait_submit_flags;
};

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

	CurrentFrameSync get_current_frame_sync_objects() noexcept;

	~SyncManager();
};