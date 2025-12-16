#pragma once

#include "Rendering/Device.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

class Swapchain : public NonCopyableClass
{
public:
	static constexpr i32 gMaxFramesInFlight = 2;

	Swapchain(Device& device, VkExtent2D extent);
	Swapchain(Device& device, VkExtent2D extent, std::shared_ptr<Swapchain> previous);
	~Swapchain();

	VkFramebuffer GetFrameBuffer(i32 index) { return mFrameBuffers[index]; }
	VkRenderPass GetRenderPass() { return mRenderPass; }
	VkImageView GetImageView(i32 index) { return mImageViews[index]; }
	u64 GetImageCount() { return mSwapChainImages.size(); }
	VkFormat GetImageFormat() { return mImageFormat; }
	VkFormat GetDepthFormat() { return mDepthFormat; }
	VkExtent2D GetExtent() { return mExtent; }
	u32 GetWidth() { return mExtent.width; }
	u32 GetHeight() { return mExtent.height; }

	f32 GetAspectRatio()
	{
		return static_cast<f32>(mExtent.width) / static_cast<f32>(mExtent.height);
	}

	VkFormat FindDepthFormat();

	VkResult AcquireNextImage(u32* imageIndex);
	VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, u32* imageIndex);

	bool CompareSwapFormats(const Swapchain& other) const;

private:
	VkFormat mImageFormat;
	VkFormat mDepthFormat;
	VkExtent2D mExtent;

	std::vector<VkFramebuffer> mFrameBuffers;
	VkRenderPass mRenderPass;

	std::vector<VkImage> mDepthImages;
	std::vector<VkDeviceMemory> mDepthImageMemorys;
	std::vector<VkImageView> mDepthImageViews;
	std::vector<VkImage> mSwapChainImages;
	std::vector<VkImageView> mImageViews;

	Device& mDevice;
	VkExtent2D mWindowExtent;

	VkSwapchainKHR mSwapChain;
	std::shared_ptr<Swapchain> mPreviousSwapchain;

	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mInFlightFences;
	std::vector<VkFence> mImagesInFlight;
	u64 mCurrentFrame = 0;

	void Init();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateDepthResources();
	void CreateRenderPass();
	void CreateFramebuffers();
	void CreateSyncObjects();

	// Helper functions
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};
