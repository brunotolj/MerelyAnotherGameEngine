#include "Vulkan/VulkanInterface.h"
#include "Vulkan/Window.h"

#include <iostream>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Vulkan
{
	Instance::Instance()
	{
		glfwInit();

		vk::ApplicationInfo appInfo
		{
			.pApplicationName = "Test",
			.applicationVersion = VK_MAKE_VERSION(0, 0, 0),
			.pEngineName = "Merely Another Game Engine",
			.engineVersion = VK_MAKE_VERSION(0, 0, 0),
			.apiVersion = vk::ApiVersion14
		};

		mage::Array<cstr> requiredLayers = GetRequiredInstanceLayers();
		mage::Array<vk::LayerProperties> supportedLayers = mContext.enumerateInstanceLayerProperties();
		
		for (cstr requiredLayer : requiredLayers)
		{
			bool layerSupported = supportedLayers.Find([requiredLayer](vk::LayerProperties const& inLayer)
				{ return strcmp(inLayer.layerName, requiredLayer) != 0; });

			mage_check(layerSupported);
		}

		mage::Array<cstr> requiredExtensions = GetRequiredInstanceExtensions();
		mage::Array<vk::ExtensionProperties> supportedExtensions = mContext.enumerateInstanceExtensionProperties();
		
		for (cstr requiredExtension : requiredExtensions)
		{
			bool extensionSupported = supportedExtensions.Find([requiredExtension](vk::ExtensionProperties const& inExtension)
				{ return strcmp(inExtension.extensionName, requiredExtension) != 0; });

			mage_check(extensionSupported);
		}

		vk::InstanceCreateInfo instanceCreateInfo
		{
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = requiredLayers.GetSize(),
			.ppEnabledLayerNames = requiredLayers.GetData(),
			.enabledExtensionCount = requiredExtensions.GetSize(),
			.ppEnabledExtensionNames = requiredExtensions.GetData()
		};

		mInstance = mContext.createInstance(instanceCreateInfo);

		VULKAN_HPP_DEFAULT_DISPATCHER.init();
		VULKAN_HPP_DEFAULT_DISPATCHER.init(*mInstance);

		SetupDebugMessenger();
	}

	Instance::~Instance()
	{
		glfwTerminate();
	}

	vk::raii::PhysicalDevice Instance::PickPhysicalDevice(PhysicalDeviceScorePredicate&& inPredicate) const
	{
		vk::raii::PhysicalDevice bestDevice = nullptr;
		u32 bestScore = 0;

		for (vk::raii::PhysicalDevice const& device : mInstance.enumeratePhysicalDevices())
		{
			u32 score = inPredicate(device);
			if (score > bestScore)
			{
				bestDevice = device;
				bestScore = score;
			}
		}

		return bestDevice;
	}

	Window Instance::CreateWindow(WindowInfo const& inWindowCreateInfo) const
	{
		return Window(inWindowCreateInfo);
	}

	vk::raii::SurfaceKHR Instance::CreateWindowSurface(Window const& inWindow) const
	{
		VkSurfaceKHR surface = nullptr;
		VkResult result = glfwCreateWindowSurface(*mInstance, inWindow.mGlfwWindow, nullptr, &surface);
		mage_check(result == VK_SUCCESS);

		return vk::raii::SurfaceKHR(mInstance, surface);
	}

	mage::Array<cstr> Instance::GetRequiredInstanceLayers() const
	{
		mage::Array<cstr> result;

		if (cEnableValidationLayers)
		{
			constexpr cstr validationLayerName = "VK_LAYER_KHRONOS_validation";
			result.Add(validationLayerName);
		}

		return result;
	}

	mage::Array<cstr> Instance::GetRequiredInstanceExtensions() const
	{
		mage::Array<cstr> result;

		u32 glfwExtensionCount = 0;
		cstr* glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (u32 i = 0; i < glfwExtensionCount; ++i)
			result.Add(glfwExtensions[i]);

		if (cEnableValidationLayers)
			result.Add(vk::EXTDebugUtilsExtensionName);

		result.Add(vk::KHRGetSurfaceCapabilities2ExtensionName);
		result.Add(vk::KHRSurfaceMaintenance1ExtensionName);

		return result;
	}

	void Instance::SetupDebugMessenger()
	{
		if (!cEnableValidationLayers)
			return;

		vk::DebugUtilsMessageSeverityFlagsEXT messageSeverityFlags(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

		vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
			| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
			| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

		vk::DebugUtilsMessengerCreateInfoEXT messengerCreateInfo
		{
			.messageSeverity = messageSeverityFlags,
			.messageType = messageTypeFlags,
			.pfnUserCallback = &VkDebugCallback
		};

		mDebugMessenger = mInstance.createDebugUtilsMessengerEXT(messengerCreateInfo);
	}

	vk::Bool32 Instance::VkDebugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT inSeverity,
		vk::DebugUtilsMessageTypeFlagsEXT inType,
		vk::DebugUtilsMessengerCallbackDataEXT const* inCallbackData,
		void*)
	{
		std::cerr << "validation layer: type " << to_string(inType) << " msg: " << inCallbackData->pMessage << std::endl;
		return vk::False;
	}
}
