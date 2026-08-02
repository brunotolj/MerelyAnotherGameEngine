#include "Vulkan/Device.h"

#include <iostream>

#include <GLFW/glfw3.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Vulkan
{
	Device::Device()
	{
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

		mVkInstance = mContext.createInstance(instanceCreateInfo);

		VULKAN_HPP_DEFAULT_DISPATCHER.init();
		VULKAN_HPP_DEFAULT_DISPATCHER.init(*mVkInstance);

		SetupDebugMessenger();

		PickPhysicalDevice();
		mage_check(mVkPhysicalDevice != nullptr);

		CreateLogicalDevice();
		mage_check(mVkDevice != nullptr);
	}

	void Device::SubmitSingleTimeCommands(SingleTimeCommandsFunction&& inFunction) const
	{
		vk::CommandBufferAllocateInfo commandBufferAllocInfo
		{
			.commandPool = mCommandPool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = 1
		};

		vk::raii::CommandBuffer commandBuffer = std::move(mVkDevice.allocateCommandBuffers(commandBufferAllocInfo).front());
		commandBuffer.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

		inFunction(commandBuffer);

		commandBuffer.end();

		vk::SubmitInfo submitInfo
		{
			.commandBufferCount = 1,
			.pCommandBuffers = &*commandBuffer
		};

		mGraphicsQueue.submit(submitInfo);
		mGraphicsQueue.waitIdle();
	}

    u32 Device::SelectMemoryType(u32 inTypeFilter, vk::MemoryPropertyFlags inProperties) const
    {
		vk::PhysicalDeviceMemoryProperties memProperties = mVkPhysicalDevice.getMemoryProperties();
		for (u32 i = 0; i < memProperties.memoryTypeCount; ++i)
			if ((inTypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & inProperties) == inProperties)
				return i;

		mage_check(false);
		return mage::InvalidIndex;
    }

	mage::Array<cstr> Device::GetRequiredInstanceLayers() const
	{
		mage::Array<cstr> result;

		if (cEnableValidationLayers)
		{
			constexpr cstr validationLayerName = "VK_LAYER_KHRONOS_validation";
			result.Add(validationLayerName);
		}

		return result;
	}

	mage::Array<cstr> Device::GetRequiredInstanceExtensions() const
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

	mage::Array<cstr> Device::GetRequiredDeviceExtensions() const
	{
		mage::Array<cstr> result;

		result.Add(vk::KHRSwapchainExtensionName);
		result.Add(vk::KHRSwapchainMaintenance1ExtensionName);
		result.Add(vk::EXTShaderObjectExtensionName);
		result.Add(vk::EXTConservativeRasterizationExtensionName);

		return result;
	}

	void Device::SetupDebugMessenger()
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

		mDebugMessenger = mVkInstance.createDebugUtilsMessengerEXT(messengerCreateInfo);
	}

	void Device::PickPhysicalDevice()
	{
		auto predicate = [this](vk::PhysicalDevice const& inDevice) -> u32
			{
				vk::PhysicalDeviceProperties deviceProperties = inDevice.getProperties();

				if (deviceProperties.apiVersion < VK_API_VERSION_1_4)
					return 0;

				mage::Array<vk::QueueFamilyProperties> queueFamilies = inDevice.getQueueFamilyProperties();
				bool supportsGraphics = false;

				for (vk::QueueFamilyProperties const& queueFamily : queueFamilies)
				{
					if ((queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlags(0))
						continue;

					supportsGraphics = true;
					break;
				}

				if (!supportsGraphics)
					return 0;

				mage::Array<vk::ExtensionProperties> supportedExtensions = inDevice.enumerateDeviceExtensionProperties();
				for (cstr requiredExtension : GetRequiredDeviceExtensions())
				{
					bool extensionSupported = supportedExtensions.Find([requiredExtension](vk::ExtensionProperties const& inExtension)
						{ return strcmp(inExtension.extensionName, requiredExtension) != 0; });

					if (!extensionSupported)
						return 0;
				}

				auto features = inDevice.getFeatures2<
					vk::PhysicalDeviceFeatures2,
					vk::PhysicalDeviceVulkan12Features,
					vk::PhysicalDeviceVulkan13Features,
					vk::PhysicalDeviceVulkan14Features,
					vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
					vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT,
					vk::PhysicalDeviceSwapchainMaintenance1FeaturesKHR>();

				{
					auto& f = features.get<vk::PhysicalDeviceFeatures2>();
					if (!f.features.sampleRateShading) return 0;
					if (!f.features.samplerAnisotropy) return 0;
				}

				{
					auto& f = features.get<vk::PhysicalDeviceVulkan12Features>();
					if (!f.bufferDeviceAddress) return 0;
				}

				{
					auto& f = features.get<vk::PhysicalDeviceVulkan13Features>();
					if (!f.shaderDemoteToHelperInvocation) return 0;
					if (!f.synchronization2) return 0;
					if (!f.dynamicRendering) return 0;
				}

				{
					auto& f = features.get<vk::PhysicalDeviceVulkan14Features>();
					if (!f.hostImageCopy) return 0;
					if (!f.pushDescriptor) return 0;
				}

				{
					auto& f = features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
					if (!f.extendedDynamicState) return 0;
				}

				{
					auto& f = features.get<vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT>();
					if (!f.extendedDynamicState3PolygonMode) return 0;
					if (!f.extendedDynamicState3RasterizationSamples) return 0;
					if (!f.extendedDynamicState3ConservativeRasterizationMode) return 0;
					if (!f.extendedDynamicState3ExtraPrimitiveOverestimationSize) return 0;
				}

				{
					auto& f = features.get<vk::PhysicalDeviceSwapchainMaintenance1FeaturesKHR>();
					if (!f.swapchainMaintenance1) return 0;
				}

				return (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) ? 2 : 1;
			};

		u32 bestScore = 0;

		for (vk::raii::PhysicalDevice const& device : mVkInstance.enumeratePhysicalDevices())
		{
			u32 score = predicate(device);
			if (score > bestScore)
			{
				mVkPhysicalDevice = device;
				bestScore = score;
			}
		}
	}

	void Device::CreateLogicalDevice()
	{
		mage::Array<vk::QueueFamilyProperties> queueFamilies = mVkPhysicalDevice.getQueueFamilyProperties();
		u32 graphicsQueueIndex = mage::InvalidIndex;

		for (u32 i = 0; i < queueFamilies.GetSize(); ++i)
		{
			vk::QueueFamilyProperties const& queueFamily = queueFamilies[i];

			if ((queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlags(0))
				continue;

			//if (!mVkPhysicalDevice.getSurfaceSupportKHR(i, mSurface))
			//	continue;

			graphicsQueueIndex = i;
			break;
		}

		mage_check(graphicsQueueIndex != mage::InvalidIndex);

		using FeatureChain = vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan12Features,
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceVulkan14Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
			vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT,
			vk::PhysicalDeviceSwapchainMaintenance1FeaturesKHR>;

		FeatureChain featureChain
		{
			{.features {
				.sampleRateShading = true,
				.samplerAnisotropy = true
			}},
			{
				.bufferDeviceAddress = true
			},
			{
				.shaderDemoteToHelperInvocation = true,
				.synchronization2 = true,
				.dynamicRendering = true
			},
			{
				.hostImageCopy = true,
				.pushDescriptor = true
			},
			{
				.extendedDynamicState = true
			},
			{
				.extendedDynamicState3PolygonMode = true,
				.extendedDynamicState3RasterizationSamples = true,
				.extendedDynamicState3ConservativeRasterizationMode = true,
				.extendedDynamicState3ExtraPrimitiveOverestimationSize = true
			},
			{
				.swapchainMaintenance1 = true
			}
		};

		float queuePriority = 1.0f;
		vk::DeviceQueueCreateInfo queueCreateInfo
		{
			.queueFamilyIndex = graphicsQueueIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		mage::Array<cstr> requiredExtensions = GetRequiredDeviceExtensions();

		vk::DeviceCreateInfo deviceCreateInfo
		{
			.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueCreateInfo,
			.enabledExtensionCount = requiredExtensions.GetSize(),
			.ppEnabledExtensionNames = requiredExtensions.GetData()
		};

		mVkDevice = mVkPhysicalDevice.createDevice(deviceCreateInfo);

		mGraphicsQueue = mVkDevice.getQueue(graphicsQueueIndex, 0);

		vk::CommandPoolCreateInfo commandPoolCreateInfo
		{
			.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			.queueFamilyIndex = graphicsQueueIndex
		};

		mCommandPool = mVkDevice.createCommandPool(commandPoolCreateInfo);
	}

	vk::Bool32 Device::VkDebugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT inSeverity,
		vk::DebugUtilsMessageTypeFlagsEXT inType,
		vk::DebugUtilsMessengerCallbackDataEXT const* inCallbackData,
		void*)
	{
		std::cerr << "validation layer: type " << to_string(inType) << " msg: " << inCallbackData->pMessage << std::endl;
		return vk::False;
	}
}
