#pragma once

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

	VkExtent2D GetExtent() const { return mSwapchain->GetExtent(); }

	f32 GetAspectRatio() const { return mSwapchain->GetAspectRatio(); }

	VkCommandBuffer GetCurrentCommandBuffer() const;

	i32 GetCurrentFrameIndex() const;

	VkRenderPass GetSwapchainRenderPass() const;

private:
	Window& mWindow;
	Device& mDevice;
	std::unique_ptr<Swapchain> mSwapchain;
	std::vector<VkCommandBuffer> mCommandBuffers;

	u32 mCurrentImageIndex = -1;
	i32 mCurrentFrameIndex = 0;
	bool mIsFrameInProgress = false;

	void CreateCommandBuffers();

	void FreeCommandBuffers();

	void RecreateSwapchain();
};
