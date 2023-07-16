#pragma once

#include "MV/MV_Window.h"
#include "NonCopyable.h"

#include <string>
#include <vector>

namespace MV
{
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	struct QueueFamilyIndices
	{
		uint32_t GraphicsFamily;
		uint32_t PresentFamily;
		bool GraphicsFamilyHasValue = false;
		bool PresentFamilyHasValue = false;
		bool IsComplete()
		{
			return GraphicsFamilyHasValue && PresentFamilyHasValue;
		}
	};

	class Device : public NonMovableClass
	{
	public:
#ifdef NDEBUG
		static constexpr bool sEnableValidationLayers = false;
#else
		static constexpr bool sEnableValidationLayers = true;
#endif

		Device(Window& window);
		~Device();

		VkCommandPool GetCommandPool() { return mCommandPool; }
		VkDevice GetDevice() { return mDevice; }
		VkSurfaceKHR GetSurface() { return mSurface; }
		VkQueue GetGraphicsQueue() { return mGraphicsQueue; }
		VkQueue GetPresentQueue() { return mPresentQueue; }

		SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(mPhysicalDevice); }
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(mPhysicalDevice); }
		VkFormat FindSupportedFormat(
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features);

		void CreateBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer& buffer,
			VkDeviceMemory& bufferMemory);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(
			VkBuffer buffer,
			VkImage image,
			uint32_t width,
			uint32_t height,
			uint32_t layerCount);

		void CreateImageWithInfo(
			const VkImageCreateInfo& imageInfo,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& imageMemory);

		VkPhysicalDeviceProperties properties;

	private:
		VkInstance mInstance;
		VkDebugUtilsMessengerEXT mDebugMessenger;
		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		Window& mWindow;
		VkCommandPool mCommandPool;

		VkDevice mDevice;
		VkSurfaceKHR mSurface;
		VkQueue mGraphicsQueue;
		VkQueue mPresentQueue;

		const std::vector<const char*> mValidationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	private:
		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateCommandPool();

		bool IsDeviceSuitable(VkPhysicalDevice device);
		std::vector<const char*> GetRequiredExtensions();
		bool CheckValidationLayerSupport();
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void HasGflwRequiredInstanceExtensions();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	};

}  // namespace lve