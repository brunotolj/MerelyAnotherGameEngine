#include "MV/MV_Device.h"

#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

using MV::Device;

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance mInstance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		mInstance,
		"vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(mInstance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance mInstance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		mInstance,
		"vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(mInstance, debugMessenger, pAllocator);
	}
}

Device::Device(Window& window) : privWindow{ window }
{
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateCommandPool();
}

Device::~Device()
{
	vkDestroyCommandPool(privDevice, privCommandPool, nullptr);
	vkDestroyDevice(privDevice, nullptr);

	if (gEnableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(privInstance, privDebugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(privInstance, privSurface, nullptr);
	vkDestroyInstance(privInstance, nullptr);
}

void Device::CreateInstance()
{
	if (gEnableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "M.A.G.E.";
	appInfo.applicationVersion = VK_API_VERSION_1_3;
	appInfo.pEngineName = "M.A.G.E.";
	appInfo.engineVersion = VK_API_VERSION_1_3;
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = GetRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (gEnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(privValidationLayers.size());
		createInfo.ppEnabledLayerNames = privValidationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &privInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create mInstance!");
	}

	HasGflwRequiredInstanceExtensions();
}

void Device::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(privInstance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::cout << "Device count: " << deviceCount << std::endl;
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(privInstance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (IsDeviceSuitable(device))
		{
			privPhysicalDevice = device;
			break;
		}
	}

	if (privPhysicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	vkGetPhysicalDeviceProperties(privPhysicalDevice, &properties);
	std::cout << "physical device: " << properties.deviceName << std::endl;
}

void Device::CreateLogicalDevice() {
	QueueFamilyIndices indices = FindQueueFamilies(privPhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.pubGraphicsFamily, indices.pubPresentFamily };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(privDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = privDeviceExtensions.data();

	// might not really be necessary anymore because device specific validation layers
	// have been deprecated
	if (gEnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(privValidationLayers.size());
		createInfo.ppEnabledLayerNames = privValidationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(privPhysicalDevice, &createInfo, nullptr, &privDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(privDevice, indices.pubGraphicsFamily, 0, &privGraphicsQueue);
	vkGetDeviceQueue(privDevice, indices.pubPresentFamily, 0, &privPresentQueue);
}

void Device::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindPhysicalQueueFamilies();

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.pubGraphicsFamily;
	poolInfo.flags =
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(privDevice, &poolInfo, nullptr, &privCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void Device::CreateSurface() 
{
	privWindow.CreateWindowSurface(privInstance, &privSurface);
}

bool Device::IsDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.pubFormats.empty() && !swapChainSupport.pubPresentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.IsComplete()
			&& extensionsSupported
			&& swapChainAdequate
			&& supportedFeatures.samplerAnisotropy;
}

void Device::PopulateDebugMessengerCreateInfo(
	VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr;  // Optional
}

void Device::SetupDebugMessenger()
{
	if (!gEnableValidationLayers)
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);
	if (CreateDebugUtilsMessengerEXT(privInstance, &createInfo, nullptr, &privDebugMessenger) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

bool Device::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : privValidationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

std::vector<const char*> Device::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (gEnableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void Device::HasGflwRequiredInstanceExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "available extensions:" << std::endl;
	std::unordered_set<std::string> available;
	for (const auto& extension : extensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
		available.insert(extension.extensionName);
	}

	std::cout << "required extensions:" << std::endl;
	auto requiredExtensions = GetRequiredExtensions();
	for (const auto& required : requiredExtensions)
	{
		std::cout << "\t" << required << std::endl;
		if (available.find(required) == available.end())
		{
			throw std::runtime_error("Missing required glfw extension");
		}
	}
}

bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(
		device,
		nullptr,
		&extensionCount,
		availableExtensions.data());

	std::set<std::string> requiredExtensions(privDeviceExtensions.begin(), privDeviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

MV::QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.pubGraphicsFamily = i;
			indices.pubGraphicsFamilyHasValue = true;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, privSurface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.pubPresentFamily = i;
			indices.pubPresentFamilyHasValue = true;
		}
		if (indices.IsComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

MV::SwapChainSupportDetails Device::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, privSurface, &details.pubCapabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, privSurface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.pubFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, privSurface, &formatCount, details.pubFormats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, privSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.pubPresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device,
			privSurface,
			&presentModeCount,
			details.pubPresentModes.data());
	}

	return details;
}

VkFormat Device::FindSupportedFormat(
	const std::vector<VkFormat>& candidates,
	VkImageTiling tiling,
	VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(privPhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

uint32_t Device::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(privPhysicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void Device::CreateBuffer(
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkBuffer& buffer,
	VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(privDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(privDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(privDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(privDevice, buffer, bufferMemory, 0);
}

VkCommandBuffer Device::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = privCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(privDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void Device::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(privGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(privGraphicsQueue);

	vkFreeCommandBuffers(privDevice, privCommandPool, 1, &commandBuffer);
}

void Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;  // Optional
	copyRegion.dstOffset = 0;  // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer);
}

void Device::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = layerCount;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region);

	EndSingleTimeCommands(commandBuffer);
}

void Device::CreateImageWithInfo(
	const VkImageCreateInfo& imageInfo,
	VkMemoryPropertyFlags properties,
	VkImage& image,
	VkDeviceMemory& imageMemory)
{
	if (vkCreateImage(privDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(privDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(privDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory!");
	}

	if (vkBindImageMemory(privDevice, image, imageMemory, 0) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to bind image memory!");
	}
}