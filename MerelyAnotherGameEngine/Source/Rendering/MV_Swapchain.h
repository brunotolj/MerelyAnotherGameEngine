#pragma once

#include "Core/NonCopyable.h"
#include "Rendering/MV_Device.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

namespace MV
{
	class Swapchain : public NonCopyableClass
	{
	public:
		static constexpr int32_t gMaxFramesInFlight = 2;

		Swapchain(Device& device, VkExtent2D extent);
		Swapchain(Device& device, VkExtent2D extent, std::shared_ptr<Swapchain> previous);
		~Swapchain();

		VkFramebuffer GetFrameBuffer(int index) { return mFrameBuffers[index]; }
		VkRenderPass GetRenderPass() { return mRenderPass; }
		VkImageView GetImageView(int index) { return mImageViews[index]; }
		size_t GetImageCount() { return mSwapChainImages.size(); }
		VkFormat GetImageFormat() { return mImageFormat; }
		VkFormat GetDepthFormat() { return mDepthFormat; }
		VkExtent2D GetExtent() { return mExtent; }
		uint32_t GetWidth() { return mExtent.width; }
		uint32_t GetHeight() { return mExtent.height; }

		float GetAspectRatio()
		{
			return static_cast<float>(mExtent.width) / static_cast<float>(mExtent.height);
		}

		VkFormat FindDepthFormat();

		VkResult AcquireNextImage(uint32_t* imageIndex);
		VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

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
		size_t mCurrentFrame = 0;

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

}
