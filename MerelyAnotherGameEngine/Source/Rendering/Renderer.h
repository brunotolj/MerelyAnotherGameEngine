#pragma once

#include "Core/Asserts.h"
#include "Rendering/Device.h"
#include "Rendering/Swapchain.h"
#include "Rendering/Window.h"

#include <array>
#include <memory>

class Renderer : public NonCopyableClass
{
public:
	Renderer(Window& window, Device& device);

	~Renderer();

	VkCommandBuffer BeginFrame();

	void EndFrame();

	void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);

	void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

	bool IsFrameInProgress() const { return mIsFrameInProgress; }

	float GetAspectRatio() const { return mSwapchain->GetAspectRatio(); }

	VkCommandBuffer GetCurrentCommandBuffer() const;

	int32_t GetCurrentFrameIndex() const;

	VkRenderPass GetSwapchainRenderPass() const;

private:
	Window& mWindow;
	Device& mDevice;
	std::unique_ptr<Swapchain> mSwapchain;
	std::vector<VkCommandBuffer> mCommandBuffers;

	uint32_t mCurrentImageIndex = -1;
	int32_t mCurrentFrameIndex = 0;
	bool mIsFrameInProgress = false;

	void CreateCommandBuffers();

	void FreeCommandBuffers();

	void RecreateSwapchain();
};
