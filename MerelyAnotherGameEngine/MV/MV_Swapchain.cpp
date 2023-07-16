#include "MV/MV_Swapchain.h"
#include "Asserts.h"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>

MV::Swapchain::Swapchain(MV::Device& device, VkExtent2D extent) :
	privDevice(device), privWindowExtent(extent)
{
	Init();
}

MV::Swapchain::Swapchain(Device& device, VkExtent2D extent, std::shared_ptr<Swapchain> previous) :
	privDevice(device), privWindowExtent(extent), privPreviousSwapchain(previous)
{
	Init();
}

MV::Swapchain::~Swapchain()
{
	for (auto imageView : privImageViews)
	{
		vkDestroyImageView(privDevice.GetDevice(), imageView, nullptr);
	}
	privImageViews.clear();

	if (privSwapChain != nullptr)
	{
		vkDestroySwapchainKHR(privDevice.GetDevice(), privSwapChain, nullptr);
		privSwapChain = nullptr;
	}

	for (int i = 0; i < privDepthImages.size(); i++)
	{
		vkDestroyImageView(privDevice.GetDevice(), privDepthImageViews[i], nullptr);
		vkDestroyImage(privDevice.GetDevice(), privDepthImages[i], nullptr);
		vkFreeMemory(privDevice.GetDevice(), privDepthImageMemorys[i], nullptr);
	}

	for (auto framebuffer : privFrameBuffers)
	{
		vkDestroyFramebuffer(privDevice.GetDevice(), framebuffer, nullptr);
	}

	vkDestroyRenderPass(privDevice.GetDevice(), privRenderPass, nullptr);

	// cleanup synchronization objects
	for (size_t i = 0; i < gMaxFramesInFlight; i++)
	{
		vkDestroySemaphore(privDevice.GetDevice(), privRenderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(privDevice.GetDevice(), privImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(privDevice.GetDevice(), privInFlightFences[i], nullptr);
	}
}

VkResult MV::Swapchain::AcquireNextImage(uint32_t* imageIndex)
{
	vkWaitForFences(
		privDevice.GetDevice(),
		1,
		&privInFlightFences[privCurrentFrame],
		VK_TRUE,
		std::numeric_limits<uint64_t>::max());

	VkResult result = vkAcquireNextImageKHR(
		privDevice.GetDevice(),
		privSwapChain,
		std::numeric_limits<uint64_t>::max(),
		privImageAvailableSemaphores[privCurrentFrame],  // must be a not signaled semaphore
		VK_NULL_HANDLE,
		imageIndex);

	return result;
}

VkResult MV::Swapchain::SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex)
{
	if (privImagesInFlight[*imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(privDevice.GetDevice(), 1, &privImagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
	}
	privImagesInFlight[*imageIndex] = privInFlightFences[privCurrentFrame];

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { privImageAvailableSemaphores[privCurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = buffers;

	VkSemaphore signalSemaphores[] = { privRenderFinishedSemaphores[privCurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(privDevice.GetDevice(), 1, &privInFlightFences[privCurrentFrame]);
	if (vkQueueSubmit(privDevice.GetGraphicsQueue(), 1, &submitInfo, privInFlightFences[privCurrentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { privSwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = imageIndex;

	auto result = vkQueuePresentKHR(privDevice.GetPresentQueue(), &presentInfo);

	privCurrentFrame = (privCurrentFrame + 1) % gMaxFramesInFlight;

	return result;
}

bool MV::Swapchain::CompareSwapFormats(const Swapchain& other) const
{
	return privImageFormat == other.privImageFormat
		&& privDepthFormat == other.privDepthFormat;
}

void MV::Swapchain::Init()
{
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateDepthResources();
	CreateFramebuffers();
	CreateSyncObjects();
}

void MV::Swapchain::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = privDevice.GetSwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.pubFormats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.pubPresentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.pubCapabilities);

	uint32_t imageCount = swapChainSupport.pubCapabilities.minImageCount + 1;
	if (swapChainSupport.pubCapabilities.maxImageCount > 0
		&& imageCount > swapChainSupport.pubCapabilities.maxImageCount)
	{
		imageCount = swapChainSupport.pubCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = privDevice.GetSurface();

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = privDevice.FindPhysicalQueueFamilies();
	uint32_t queueFamilyIndices[] = { indices.pubGraphicsFamily, indices.pubPresentFamily };

	if (indices.pubGraphicsFamily != indices.pubPresentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;      // Optional
		createInfo.pQueueFamilyIndices = nullptr;  // Optional
	}

	createInfo.preTransform = swapChainSupport.pubCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = privPreviousSwapchain != nullptr ? privPreviousSwapchain->privSwapChain : nullptr;

	if (vkCreateSwapchainKHR(privDevice.GetDevice(), &createInfo, nullptr, &privSwapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	// we only specified a minimum number of images in the swap chain, so the implementation is
	// allowed to create a swap chain with more. That's why we'll first query the final number of
	// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
	// retrieve the handles.
	vkGetSwapchainImagesKHR(privDevice.GetDevice(), privSwapChain, &imageCount, nullptr);
	privSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(privDevice.GetDevice(), privSwapChain, &imageCount, privSwapChainImages.data());

	privImageFormat = surfaceFormat.format;
	privExtent = extent;
}

void MV::Swapchain::CreateImageViews()
{
	privImageViews.resize(privSwapChainImages.size());
	for (size_t i = 0; i < privSwapChainImages.size(); i++) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = privSwapChainImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = privImageFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(privDevice.GetDevice(), &viewInfo, nullptr, &privImageViews[i]) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}
	}
}

void MV::Swapchain::CreateRenderPass()
{
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = GetImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstSubpass = 0;
	dependency.dstStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(privDevice.GetDevice(), &renderPassInfo, nullptr, &privRenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void MV::Swapchain::CreateFramebuffers()
{
	privFrameBuffers.resize(GetImageCount());
	for (size_t i = 0; i < GetImageCount(); i++)
	{
		std::array<VkImageView, 2> attachments = { privImageViews[i], privDepthImageViews[i] };

		VkExtent2D swapChainExtent = GetExtent();
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = privRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		check(vkCreateFramebuffer(privDevice.GetDevice(), &framebufferInfo, nullptr, &privFrameBuffers[i]) == VK_SUCCESS);
	}
}

void MV::Swapchain::CreateDepthResources()
{
	VkFormat depthFormat = FindDepthFormat();
	privDepthFormat = depthFormat;

	VkExtent2D swapChainExtent = GetExtent();

	privDepthImages.resize(GetImageCount());
	privDepthImageMemorys.resize(GetImageCount());
	privDepthImageViews.resize(GetImageCount());

	for (int i = 0; i < privDepthImages.size(); i++)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = swapChainExtent.width;
		imageInfo.extent.height = swapChainExtent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = 0;

		privDevice.CreateImageWithInfo(
			imageInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			privDepthImages[i],
			privDepthImageMemorys[i]);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = privDepthImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		check(vkCreateImageView(privDevice.GetDevice(), &viewInfo, nullptr, &privDepthImageViews[i]) == VK_SUCCESS);
	}
}

void MV::Swapchain::CreateSyncObjects()
{
	privImageAvailableSemaphores.resize(gMaxFramesInFlight);
	privRenderFinishedSemaphores.resize(gMaxFramesInFlight);
	privInFlightFences.resize(gMaxFramesInFlight);
	privImagesInFlight.resize(GetImageCount(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < gMaxFramesInFlight; i++)
	{
		check(vkCreateSemaphore(privDevice.GetDevice(), &semaphoreInfo, nullptr, &privImageAvailableSemaphores[i]) == VK_SUCCESS);
		check(vkCreateSemaphore(privDevice.GetDevice(), &semaphoreInfo, nullptr, &privRenderFinishedSemaphores[i]) == VK_SUCCESS);
		check(vkCreateFence(privDevice.GetDevice(), &fenceInfo, nullptr, &privInFlightFences[i]) == VK_SUCCESS);
	}
}

VkSurfaceFormatKHR MV::Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR MV::Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D MV::Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = privWindowExtent;
		actualExtent.width = std::max(
			capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(
			capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

VkFormat MV::Swapchain::FindDepthFormat()
{
	return privDevice.FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
