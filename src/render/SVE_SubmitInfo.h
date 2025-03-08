#pragma once

#include "core.h"

class SveCommandList {
private:
	VkCommandPool pool;
	VkCommandBuffer buffer;
	std::vector<VkFence> fences;
	std::vector<VkPipelineStageFlags> waitStages;
	std::vector<VkSemaphore> waitSemaphores;
	std::vector<VkSemaphore> signalSemaphores;
	uint32_t queueIndex;

public:
	SveCommandList(uint32_t queueIndex, VkCommandBufferUsageFlags commandUsage);
	~SveCommandList();
	void addWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags waitStage);
	void addSignalSemaphore(VkSemaphore semaphore);
	void addFence(VkFence fence);
	void startRecording();
	void getCommandBuffer();
	void endRecording();
	void submit(VkQueue queue);
};