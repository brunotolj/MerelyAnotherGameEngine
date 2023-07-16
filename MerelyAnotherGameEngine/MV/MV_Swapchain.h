#pragma once

#include "MV/MV_Device.h"
#include "NonCopyable.h"

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

		VkFramebuffer GetFrameBuffer(int index) { return privFrameBuffers[index]; }
		VkRenderPass GetRenderPass() { return privRenderPass; }
		VkImageView GetImageView(int index) { return privImageViews[index]; }
		size_t GetImageCount() { return privSwapChainImages.size(); }
		VkFormat GetImageFormat() { return privImageFormat; }
		VkFormat GetDepthFormat() { return privDepthFormat; }
		VkExtent2D GetExtent() { return privExtent; }
		uint32_t GetWidth() { return privExtent.width; }
		uint32_t GetHeight() { return privExtent.height; }

		float GetAspectRatio()
		{
			return static_cast<float>(privExtent.width) / static_cast<float>(privExtent.height);
		}

		VkFormat FindDepthFormat();

		VkResult AcquireNextImage(uint32_t* imageIndex);
		VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

		bool CompareSwapFormats(const Swapchain& other) const;

	private:
		VkFormat privImageFormat;
		VkFormat privDepthFormat;
		VkExtent2D privExtent;

		std::vector<VkFramebuffer> privFrameBuffers;
		VkRenderPass privRenderPass;

		std::vector<VkImage> privDepthImages;
		std::vector<VkDeviceMemory> privDepthImageMemorys;
		std::vector<VkImageView> privDepthImageViews;
		std::vector<VkImage> privSwapChainImages;
		std::vector<VkImageView> privImageViews;

		Device& privDevice;
		VkExtent2D privWindowExtent;

		VkSwapchainKHR privSwapChain;
		std::shared_ptr<Swapchain> privPreviousSwapchain;

		std::vector<VkSemaphore> privImageAvailableSemaphores;
		std::vector<VkSemaphore> privRenderFinishedSemaphores;
		std::vector<VkFence> privInFlightFences;
		std::vector<VkFence> privImagesInFlight;
		size_t privCurrentFrame = 0;

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
