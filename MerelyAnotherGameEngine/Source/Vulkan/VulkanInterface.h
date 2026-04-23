#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	class Window;
	struct WindowInfo;

	class Instance : public NonMovableClass
	{
	public:
		Instance();
		~Instance();

		using PhysicalDeviceScorePredicate = std::function<u32(vk::PhysicalDevice const&)>;
		vk::raii::PhysicalDevice PickPhysicalDevice(PhysicalDeviceScorePredicate&& inPredicate) const;

		Window CreateWindow(WindowInfo const& inWindowCreateInfo) const;
		vk::raii::SurfaceKHR CreateWindowSurface(Window const& inWindow) const;

		mage::Array<cstr> GetRequiredInstanceLayers() const;
		mage::Array<cstr> GetRequiredInstanceExtensions() const;

	private:
		void SetupDebugMessenger();

		vk::raii::Context mContext;
		vk::raii::Instance mInstance = nullptr;
		vk::raii::DebugUtilsMessengerEXT mDebugMessenger = nullptr;

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
