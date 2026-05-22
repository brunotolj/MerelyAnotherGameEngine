#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	class Device : public NonMovableClass
	{
	public:
		Device();
		~Device();

		vk::raii::Instance const& GetVkInstance() const;
		vk::raii::PhysicalDevice const& GetVkPhysicalDevice() const;
		vk::raii::Device const& GetVkDevice() const;
		vk::raii::Queue const& GetGraphicsQueue() const;
		vk::raii::CommandPool const& GetCommandPool() const;

		using SingleTimeCommandsFunction = std::function<void(vk::CommandBuffer)>;
		void SubmitSingleTimeCommands(SingleTimeCommandsFunction&& inFunction) const;

		u32 SelectMemoryType(u32 inTypeFilter, vk::MemoryPropertyFlags inProperties) const;

		mage::Array<cstr> GetRequiredInstanceLayers() const;
		mage::Array<cstr> GetRequiredInstanceExtensions() const;
		mage::Array<cstr> GetRequiredDeviceExtensions() const;

	private:
		void SetupDebugMessenger();

		void PickPhysicalDevice();
		void CreateLogicalDevice();

		vk::raii::Context mContext;
		vk::raii::Instance mVkInstance = nullptr;

		vk::raii::DebugUtilsMessengerEXT mDebugMessenger = nullptr;

		vk::raii::PhysicalDevice mVkPhysicalDevice = nullptr;
		vk::raii::Device mVkDevice = nullptr;

		vk::raii::Queue mGraphicsQueue = nullptr;
		vk::raii::CommandPool mCommandPool = nullptr;

		static VKAPI_ATTR vk::Bool32 VKAPI_CALL VkDebugCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT inSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT inType,
			vk::DebugUtilsMessengerCallbackDataEXT const* inCallbackData,
			void*);

#ifdef MAGE_DEBUG
		static constexpr bool cEnableValidationLayers = true;
#else
		static constexpr bool cEnableValidationLayers = false;
#endif
	};
}
