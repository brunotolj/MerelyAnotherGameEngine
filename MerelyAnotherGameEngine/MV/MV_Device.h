#pragma once

#include "MV/MV_Window.h"
#include "NonCopyable.h"

#include <string>
#include <vector>

#ifdef NDEBUG
static constexpr bool gEnableValidationLayers = false;
#else
static constexpr bool gEnableValidationLayers = true;
#endif

namespace MV
{
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR pubCapabilities;
		std::vector<VkSurfaceFormatKHR> pubFormats;
		std::vector<VkPresentModeKHR> pubPresentModes;
	};

	struct QueueFamilyIndices
	{
		uint32_t pubGraphicsFamily;
		uint32_t pubPresentFamily;
		bool pubGraphicsFamilyHasValue = false;
		bool pubPresentFamilyHasValue = false;

		bool IsComplete()
		{
			return pubGraphicsFamilyHasValue && pubPresentFamilyHasValue;
		}
	};

	class Device : public NonMovableClass
	{
	public:
		Device(Window& window);
		~Device();

		VkCommandPool GetCommandPool() { return privCommandPool; }
		VkDevice GetDevice() { return privDevice; }
		VkSurfaceKHR GetSurface() { return privSurface; }
		VkQueue GetGraphicsQueue() { return privGraphicsQueue; }
		VkQueue GetPresentQueue() { return privPresentQueue; }

		SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(privPhysicalDevice); }
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(privPhysicalDevice); }
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
		VkInstance privInstance;
		VkDebugUtilsMessengerEXT privDebugMessenger;
		VkPhysicalDevice privPhysicalDevice = VK_NULL_HANDLE;
		Window& privWindow;
		VkCommandPool privCommandPool;

		VkDevice privDevice;
		VkSurfaceKHR privSurface;
		VkQueue privGraphicsQueue;
		VkQueue privPresentQueue;

		const std::vector<const char*> privValidationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> privDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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
}
