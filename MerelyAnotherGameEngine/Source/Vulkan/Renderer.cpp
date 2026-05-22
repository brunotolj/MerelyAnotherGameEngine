#include "Vulkan/Renderer.h"
#include "Engine/Engine.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Window.h"

namespace Vulkan
{
	Renderer::Renderer(Window& inWindow)
	{
		vk::raii::Device const& device = gEngine->mVulkanDevice.GetVkDevice();
		vk::raii::PhysicalDevice const& physicalDevice = gEngine->mVulkanDevice.GetVkPhysicalDevice();

		vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice.getProperties();
		VkFlags supportedSampleCounts = VkFlags(physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts);

		while (VkFlags(supportedSampleCounts) & (VkFlags(msaaSamples) << 1))
			msaaSamples = vk::SampleCountFlagBits(VkFlags(msaaSamples) << 1);

		mSurface = inWindow.CreateVkSurface(gEngine->mVulkanDevice.GetVkInstance());
		mage_check(mSurface != nullptr);

		vk::CommandBufferAllocateInfo commandBufferAllocInfo
		{
			.commandPool = gEngine->mVulkanDevice.GetCommandPool(),
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = cMaxFramesInFlight
		};

		mCommandBuffers = device.allocateCommandBuffers(commandBufferAllocInfo);

		for (u32 i = 0; i < cMaxFramesInFlight; ++i)
		{
			mPresentCompleteSemaphores.Add(device.createSemaphore({}));
			mRenderFinishedSemaphores.Add(device.createSemaphore({}));
			mDrawFences.Add(device.createFence({ .flags = vk::FenceCreateFlagBits::eSignaled }));
			mPresentFences.Add(device.createFence({ .flags = vk::FenceCreateFlagBits::eSignaled }));
		}

		mWindowSize = inWindow.GetSize();
		inWindow.SetResizedCallback([this](i32 inNewWidth, i32 inNewHeight)
			{
				mWasWindowResized = true;
				mWindowSize.width = u32(inNewWidth);
				mWindowSize.height = u32(inNewHeight);
			});

		RecreateSwapchain();
	}

	void Renderer::RenderFrame(Renderer::RenderFrameFunction&& inFunction)
	{
		vk::Result result;

		if (mWasWindowResized)
		{
			mWasWindowResized = false;
			RecreateSwapchain();
		}

		vk::raii::CommandBuffer& commandBuffer = mCommandBuffers[mCurrentFrameIndex];
		vk::Semaphore currentPresentCompleteSemaphore = mPresentCompleteSemaphores[mCurrentFrameIndex];
		vk::Semaphore currentRenderFinishedSemaphore = mRenderFinishedSemaphores[mCurrentFrameIndex];
		vk::Fence currentDrawFence = mDrawFences[mCurrentFrameIndex];
		vk::Fence currentPresentFence = mPresentFences[mCurrentFrameIndex];

		vk::raii::Device const& device = gEngine->mVulkanDevice.GetVkDevice();
		vk::raii::Queue const& graphicsQueue = gEngine->mVulkanDevice.GetGraphicsQueue();

		result = device.waitForFences(currentDrawFence, vk::True, UINT64_MAX);
		mage_check(result == vk::Result::eSuccess);

		vk::ResultValue<u32> acquireNextImageResult = mSwapchain.acquireNextImage(UINT64_MAX, currentPresentCompleteSemaphore, nullptr);
		mage_check(acquireNextImageResult.result == vk::Result::eSuccess);

		mCurrentImageIndex = acquireNextImageResult.value;
		vk::Image currentImage = mSwapchainImages[mCurrentImageIndex];
		vk::ImageView currentImageView = mSwapchainImageViews[mCurrentImageIndex];

		device.resetFences(currentDrawFence);

		commandBuffer.begin({});

		{
			Image::TransitionLayoutParams transitionParams
			{
				.SrcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				.SrcAccessMask = {},
				.DstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				.DstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
				.OldLayout = vk::ImageLayout::eUndefined,
				.NewLayout = vk::ImageLayout::eColorAttachmentOptimal
			};

			Image::TransitionLayout(commandBuffer, currentImage, transitionParams, vk::ImageAspectFlagBits::eColor);
		}

		{
			Image::TransitionLayoutParams transitionParams
			{
				.SrcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				.SrcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
				.DstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				.DstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
				.OldLayout = vk::ImageLayout::eUndefined,
				.NewLayout = vk::ImageLayout::eColorAttachmentOptimal
			};

			mColorImage.TransitionLayout(commandBuffer, transitionParams);
		}

		{
			Image::TransitionLayoutParams transitionParams
			{
				.SrcStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				.SrcAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				.DstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				.DstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				.OldLayout = vk::ImageLayout::eUndefined,
				.NewLayout = vk::ImageLayout::eDepthAttachmentOptimal
			};

			mDepthImage.TransitionLayout(commandBuffer, transitionParams);
		}

		vk::RenderingAttachmentInfo colorAttachmentInfo
		{
			.imageView = mColorImage.GetVkImageView(),
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.resolveMode = vk::ResolveModeFlagBits::eAverage,
			.resolveImageView = currentImageView,
			.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = vk::ClearColorValue(0.1f, 0.1f, 0.15f, 1.0f)
		};

		vk::RenderingAttachmentInfo depthAttachmentInfo
		{
			.imageView = mDepthImage.GetVkImageView(),
			.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eDontCare,
			.clearValue = vk::ClearDepthStencilValue(1.0f, 0)
		};

		vk::RenderingInfo renderingInfo
		{
			.renderArea
			{
				.offset = {0, 0},
				.extent = mSwapchainExtent
			},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentInfo,
			.pDepthAttachment = &depthAttachmentInfo
		};

		commandBuffer.beginRendering(renderingInfo);

		InitializeDynamicState(commandBuffer);

		inFunction({ commandBuffer, mSwapchainExtent, mCurrentFrameIndex });

		commandBuffer.endRendering();

		{
			Image::TransitionLayoutParams transitionParams
			{
				.SrcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				.SrcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
				.DstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
				.DstAccessMask = {},
				.OldLayout = vk::ImageLayout::eColorAttachmentOptimal,
				.NewLayout = vk::ImageLayout::ePresentSrcKHR
			};

			Image::TransitionLayout(commandBuffer, currentImage, transitionParams, vk::ImageAspectFlagBits::eColor);
		}

		commandBuffer.end();

		result = device.waitForFences(currentPresentFence, vk::True, UINT64_MAX);
		mage_check(result == vk::Result::eSuccess);

		device.resetFences(currentPresentFence);

		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		vk::SubmitInfo submitInfo
		{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &currentPresentCompleteSemaphore,
			.pWaitDstStageMask = &waitDestinationStageMask,
			.commandBufferCount = 1,
			.pCommandBuffers = &*commandBuffer,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &currentRenderFinishedSemaphore
		};

		graphicsQueue.submit(submitInfo, currentDrawFence);

		vk::StructureChain<vk::PresentInfoKHR, vk::SwapchainPresentFenceInfoKHR> presentInfoChain
		{
			{
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &currentRenderFinishedSemaphore,
				.swapchainCount = 1,
				.pSwapchains = &*mSwapchain,
				.pImageIndices = &mCurrentImageIndex
			},
			{
				.swapchainCount = 1,
				.pFences = &currentPresentFence
			}
		};

		result = graphicsQueue.presentKHR(presentInfoChain.get<vk::PresentInfoKHR>());
		mage_check(result == vk::Result::eSuccess);

		mCurrentImageIndex = u32(-1);
		mCurrentFrameIndex = (mCurrentFrameIndex + 1) % cMaxFramesInFlight;
	}

	void Renderer::RecreateSwapchain()
	{
		vk::raii::PhysicalDevice const& physicalDevice = gEngine->mVulkanDevice.GetVkPhysicalDevice();
		vk::raii::Device const& device = gEngine->mVulkanDevice.GetVkDevice();

		device.waitIdle();

		vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(mSurface);
		mage::Array<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(mSurface);
		mage::Array<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(mSurface);

		mSwapchainSurfaceFormat = ChooseSwapchainFormat(formats);
		mSwapchainExtent = ChooseSwapchainExtent(capabilities, mWindowSize);

		vk::SwapchainCreateInfoKHR swapchainCreateInfo
		{
			.surface = mSurface,
			.minImageCount = ChooseSwapchainMinImageCount(capabilities),
			.imageFormat = mSwapchainSurfaceFormat.format,
			.imageColorSpace = mSwapchainSurfaceFormat.colorSpace,
			.imageExtent = mSwapchainExtent,
			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.imageSharingMode = vk::SharingMode::eExclusive,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = ChooseSwapchainPresentMode(presentModes),
			.clipped = true,
			.oldSwapchain = mSwapchain
		};

		mSwapchainImageViews.Empty();
		mSwapchain = device.createSwapchainKHR(swapchainCreateInfo);
		mSwapchainImages = mSwapchain.getImages();

		vk::ImageViewCreateInfo imageViewCreateInfo
		{
			.viewType = vk::ImageViewType::e2D,
			.format = mSwapchainSurfaceFormat.format,
			.subresourceRange
			{
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.levelCount = 1,
				.layerCount = 1
			}
		};

		for (vk::Image& image : mSwapchainImages)
		{
			imageViewCreateInfo.image = image;
			mSwapchainImageViews.Add(device.createImageView(imageViewCreateInfo));
		}

		Image::CreateInfo colorImageCreateInfo
		{
			.Size = { mSwapchainExtent.width, mSwapchainExtent.height, 1 },
			.Format = mSwapchainSurfaceFormat.format,
			.AspectFlags = vk::ImageAspectFlagBits::eColor,
			.UsageFlags = vk::ImageUsageFlagBits::eColorAttachment,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
			.SampleCount = msaaSamples
		};

		mColorImage.Create(colorImageCreateInfo);

		Image::CreateInfo depthImageCreateInfo
		{
			.Size = { mSwapchainExtent.width, mSwapchainExtent.height, 1 },
			.Format = vk::Format::eD32Sfloat,
			.AspectFlags = vk::ImageAspectFlagBits::eDepth,
			.UsageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
			.SampleCount = msaaSamples
		};

		mDepthImage.Create(depthImageCreateInfo);
	}

	void Renderer::InitializeDynamicState(vk::CommandBuffer inCommandBuffer)
	{
		inCommandBuffer.setCullMode(vk::CullModeFlagBits::eBack);
		inCommandBuffer.setFrontFace(vk::FrontFace::eClockwise);
		inCommandBuffer.setViewportWithCount(vk::Viewport(0.0f, 0.0f, f32(mSwapchainExtent.width), f32(mSwapchainExtent.height), 0.0f, 1.0f));
		inCommandBuffer.setScissorWithCount(vk::Rect2D(vk::Offset2D(0, 0), mSwapchainExtent));
		inCommandBuffer.setDepthTestEnable(vk::True);
		inCommandBuffer.setPolygonModeEXT(vk::PolygonMode::eFill);
		inCommandBuffer.setRasterizationSamplesEXT(msaaSamples);
	}

	vk::SurfaceFormatKHR Renderer::ChooseSwapchainFormat(mage::Array<vk::SurfaceFormatKHR> const& inFormats) const
	{
		vk::SurfaceFormatKHR preferredFormat
		{
			.format = vk::Format::eB8G8R8A8Srgb,
			.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear
		};

		return inFormats.Contains(preferredFormat)
			? preferredFormat
			: inFormats[0];
	}

	vk::PresentModeKHR Renderer::ChooseSwapchainPresentMode(mage::Array<vk::PresentModeKHR> const& inPresentModes) const
	{
		return inPresentModes.Contains(vk::PresentModeKHR::eMailbox)
			? vk::PresentModeKHR::eMailbox
			: vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D Renderer::ChooseSwapchainExtent(vk::SurfaceCapabilitiesKHR const& inCapabilities, vk::Extent2D inWindowExtent) const
	{
		if (inCapabilities.currentExtent.width != u32(-1))
			return inCapabilities.currentExtent;

		return
		{
			std::clamp<u32>(inWindowExtent.width, inCapabilities.minImageExtent.width, inCapabilities.maxImageExtent.width),
			std::clamp<u32>(inWindowExtent.height, inCapabilities.minImageExtent.height, inCapabilities.maxImageExtent.height)
		};
	}

	u32 Renderer::ChooseSwapchainMinImageCount(vk::SurfaceCapabilitiesKHR const& inCapabilities) const
	{
		return (inCapabilities.minImageCount == inCapabilities.maxImageCount)
			? inCapabilities.minImageCount
			: inCapabilities.minImageCount + 1;
	}
}
