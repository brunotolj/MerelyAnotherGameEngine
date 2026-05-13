#include "Vulkan/Renderer.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/VulkanInterface.h"
#include "Vulkan/Window.h"

namespace Vulkan
{
	Renderer::Renderer(Instance const& inInstance, Window& inWindow)
	{
		mPhysicalDevice = PickPhysicalDevice(inInstance);
		mage_check(mPhysicalDevice != nullptr);

		vk::PhysicalDeviceProperties physicalDeviceProperties = mPhysicalDevice.getProperties();
		VkFlags supportedSampleCounts = VkFlags(physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts);

		while (VkFlags(supportedSampleCounts) & (VkFlags(msaaSamples) << 1))
			msaaSamples = vk::SampleCountFlagBits(VkFlags(msaaSamples) << 1);

		mSurface = inInstance.CreateWindowSurface(inWindow);
		mage_check(mSurface != nullptr);

		SetupGraphicsQueue();

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

		result = mDevice.waitForFences(currentDrawFence, vk::True, UINT64_MAX);
		mage_check(result == vk::Result::eSuccess);

		vk::ResultValue<u32> acquireNextImageResult = mSwapchain.acquireNextImage(UINT64_MAX, currentPresentCompleteSemaphore, nullptr);
		mage_check(acquireNextImageResult.result == vk::Result::eSuccess);

		mCurrentImageIndex = acquireNextImageResult.value;
		vk::Image currentImage = mSwapchainImages[mCurrentImageIndex];
		vk::ImageView currentImageView = mSwapchainImageViews[mCurrentImageIndex];

		mDevice.resetFences(currentDrawFence);

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
			.imageView = mColorImage.mImageView,
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
			.imageView = mDepthImage.mImageView,
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

		result = mDevice.waitForFences(currentPresentFence, vk::True, UINT64_MAX);
		mage_check(result == vk::Result::eSuccess);

		mDevice.resetFences(currentPresentFence);

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

		mGraphicsQueue.submit(submitInfo, currentDrawFence);

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

		result = mGraphicsQueue.presentKHR(presentInfoChain.get<vk::PresentInfoKHR>());
		mage_check(result == vk::Result::eSuccess);

		mCurrentImageIndex = u32(-1);
		mCurrentFrameIndex = (mCurrentFrameIndex + 1) % cMaxFramesInFlight;
	}

	void Renderer::SubmitSingleTimeCommands(SingleTimeCommandsFunction&& inFunction) const
	{
		vk::CommandBufferAllocateInfo commandBufferAllocInfo
		{
			.commandPool = mCommandPool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = 1
		};

		vk::raii::CommandBuffer commandBuffer = std::move(mDevice.allocateCommandBuffers(commandBufferAllocInfo).front());
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

	void Renderer::WaitIdle() const
	{
		mDevice.waitIdle();
	}

	Pipeline Renderer::CreatePipeline(PipelineCreateInfo const& inPipelineCreateInfo) const
	{
		Pipeline result;

		mage_check(inPipelineCreateInfo.ShaderCode.GetSize());

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
		{
			.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptor,
			.bindingCount = inPipelineCreateInfo.DescriptorSetBindings.GetSize(),
			.pBindings = inPipelineCreateInfo.DescriptorSetBindings.GetData()
		};

		result.mDescriptorSetLayout = mDevice.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo
		{
			.setLayoutCount = 1,
			.pSetLayouts = &*result.mDescriptorSetLayout,
			.pushConstantRangeCount = inPipelineCreateInfo.PushConstantRanges.GetSize(),
			.pPushConstantRanges = inPipelineCreateInfo.PushConstantRanges.GetData()
		};

		result.mPipelineLayout = mDevice.createPipelineLayout(pipelineLayoutCreateInfo);

		vk::ShaderModuleCreateInfo shaderModuleCreateInfo
		{
			.codeSize = inPipelineCreateInfo.ShaderCode.GetSize() * sizeof(u32),
			.pCode = inPipelineCreateInfo.ShaderCode.GetData()
		};

		vk::raii::ShaderModule shaderModule{ mDevice, shaderModuleCreateInfo };

		mage::Array<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos;

		for (PipelineShaderStageInfo const& shaderStageInfo : inPipelineCreateInfo.ShaderStages)
			shaderStageCreateInfos.Add
			({
				.stage = shaderStageInfo.Stage,
				.module = shaderModule,
				.pName = shaderStageInfo.EntryPoint.GetCString()
			});

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo
		{
			.vertexBindingDescriptionCount = inPipelineCreateInfo.InputBindingDescriptions.GetSize(),
			.pVertexBindingDescriptions = inPipelineCreateInfo.InputBindingDescriptions.GetData(),
			.vertexAttributeDescriptionCount = inPipelineCreateInfo.InputAttributeDescriptions.GetSize(),
			.pVertexAttributeDescriptions = inPipelineCreateInfo.InputAttributeDescriptions.GetData()
		};

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo {};

		vk::PipelineViewportStateCreateInfo viewportInfo {};

		vk::PipelineRasterizationStateCreateInfo rasterizationInfo
		{
			.frontFace = vk::FrontFace::eClockwise,
			.lineWidth = 1.0f
		};

		vk::PipelineMultisampleStateCreateInfo multisampleInfo
		{
			.sampleShadingEnable = vk::True,
			.minSampleShading = 0.2f
		};

		vk::PipelineColorBlendAttachmentState colorBlendAttachmentState
		{
			.blendEnable = vk::True,
			.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
			.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.colorBlendOp = vk::BlendOp::eAdd,
			.srcAlphaBlendFactor = vk::BlendFactor::eOne,
			.dstAlphaBlendFactor = vk::BlendFactor::eZero,
			.alphaBlendOp = vk::BlendOp::eAdd,
			.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
		};

		vk::PipelineDepthStencilStateCreateInfo depthStencilInfo
		{
			.depthCompareOp = vk::CompareOp::eLess
		};

		vk::PipelineColorBlendStateCreateInfo colorBlendInfo
		{
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachmentState
		};

		mage::Array<vk::DynamicState> dynamicStates
		{
			vk::DynamicState::eCullMode,
			vk::DynamicState::eFrontFace,
			vk::DynamicState::ePrimitiveTopology,
			vk::DynamicState::eViewportWithCount,
			vk::DynamicState::eScissorWithCount,
			vk::DynamicState::eDepthTestEnable,
			vk::DynamicState::eDepthWriteEnable,
			vk::DynamicState::ePolygonModeEXT,
			vk::DynamicState::eRasterizationSamplesEXT,
			vk::DynamicState::eConservativeRasterizationModeEXT,
			vk::DynamicState::eExtraPrimitiveOverestimationSizeEXT
		};

		vk::PipelineDynamicStateCreateInfo dynamicStateInfo
		{
			.dynamicStateCount = dynamicStates.GetSize(),
			.pDynamicStates = dynamicStates.GetData()
		};

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain
		{
			{
				.stageCount = shaderStageCreateInfos.GetSize(),
				.pStages = shaderStageCreateInfos.GetData(),
				.pVertexInputState = &vertexInputInfo,
				.pInputAssemblyState = &inputAssemblyInfo,
				.pViewportState = &viewportInfo,
				.pRasterizationState = &rasterizationInfo,
				.pMultisampleState = &multisampleInfo,
				.pDepthStencilState = &depthStencilInfo,
				.pColorBlendState = &colorBlendInfo,
				.pDynamicState = &dynamicStateInfo,
				.layout = result.mPipelineLayout,
				.renderPass = nullptr
			},
			{
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &mSwapchainSurfaceFormat.format,
				.depthAttachmentFormat = vk::Format::eD32Sfloat
			}
		};

		result.mVkPipeline = mDevice.createGraphicsPipeline(nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());

		return result;
	}

	Buffer Renderer::CreateBuffer(BufferCreateInfo const& inBufferCreateInfo) const
	{
		Buffer result = nullptr;

		vk::BufferCreateInfo bufferCreateInfo
		{
			.size = inBufferCreateInfo.Size,
			.usage = inBufferCreateInfo.UsageFlags | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			.sharingMode = vk::SharingMode::eExclusive
		};

		result.mVkBuffer = mDevice.createBuffer(bufferCreateInfo);
		result.mBufferSize = inBufferCreateInfo.Size;

		vk::MemoryRequirements memRequirements = result.mVkBuffer.getMemoryRequirements();
		u32 memoryTypeIndex = SelectMemoryType(memRequirements.memoryTypeBits, inBufferCreateInfo.MemoryFlags);
		
		vk::MemoryAllocateFlagsInfo memoryAllocFlagsInfo{ .flags = vk::MemoryAllocateFlagBits::eDeviceAddress };

		vk::MemoryAllocateInfo memoryAllocInfo
		{
			.pNext = memoryAllocFlagsInfo,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = memoryTypeIndex
		};

		result.mDeviceMemory = mDevice.allocateMemory(memoryAllocInfo);
		result.mVkBuffer.bindMemory(result.mDeviceMemory, 0);

		result.mDeviceAddress = mDevice.getBufferAddress({ .buffer = result.mVkBuffer });

		return result;
	}

	Image Renderer::CreateImage(ImageCreateInfo const& inImageCreateInfo) const
	{
		Image result = nullptr;

		vk::ImageCreateInfo imageCreateInfo
		{
			.imageType = vk::ImageType::e2D,
			.format = inImageCreateInfo.Format,
			.extent = inImageCreateInfo.Size,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = inImageCreateInfo.SampleCount,
			.tiling = vk::ImageTiling::eOptimal,
			.usage = inImageCreateInfo.UsageFlags,
			.sharingMode = vk::SharingMode::eExclusive
		};

		result.mVkImage = mDevice.createImage(imageCreateInfo);
		result.mImageSize = inImageCreateInfo.Size;
		result.mAspectMask = inImageCreateInfo.AspectFlags;

		vk::MemoryRequirements memRequirements = result.mVkImage.getMemoryRequirements();
		u32 memoryTypeIndex = SelectMemoryType(memRequirements.memoryTypeBits, inImageCreateInfo.MemoryFlags);

		vk::MemoryAllocateInfo memoryAllocInfo
		{
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = memoryTypeIndex
		};

		result.mDeviceMemory = mDevice.allocateMemory(memoryAllocInfo);
		result.mVkImage.bindMemory(result.mDeviceMemory, 0);

		vk::ImageViewCreateInfo imageViewCreateInfo
		{
			.image = result.mVkImage,
			.viewType = vk::ImageViewType::e2D,
			.format = inImageCreateInfo.Format,
			.subresourceRange
			{
				.aspectMask = inImageCreateInfo.AspectFlags,
				.levelCount = 1,
				.layerCount = 1
			}
		};

		result.mImageView = mDevice.createImageView(imageViewCreateInfo);

		return result;
	}

	vk::raii::Sampler Renderer::CreateImageSampler(vk::SamplerCreateInfo inSamplerCreateInfo) const
	{
		inSamplerCreateInfo.maxAnisotropy = mPhysicalDevice.getProperties().limits.maxSamplerAnisotropy;
		return mDevice.createSampler(inSamplerCreateInfo);
	}

	void Renderer::CopyMemoryToImage(void* inSrcMemory, Image& inDstImage, vk::ImageLayout inImageLayout) const
	{
		vk::HostImageLayoutTransitionInfo layoutTransitionInfo
		{
			.image = inDstImage.mVkImage,
			.oldLayout = vk::ImageLayout::eUndefined,
			.newLayout = inImageLayout,
			.subresourceRange
			{
				   .aspectMask = inDstImage.mAspectMask,
				   .baseMipLevel = 0,
				   .levelCount = 1,
				   .baseArrayLayer = 0,
				   .layerCount = 1
			}
		};

		mDevice.transitionImageLayout(layoutTransitionInfo);
		inDstImage.mImageLayout = inImageLayout;

		vk::MemoryToImageCopy copy
		{
			.pHostPointer = inSrcMemory,
			.imageSubresource
			{
				   .aspectMask = inDstImage.mAspectMask,
				   .mipLevel = 0,
				   .baseArrayLayer = 0,
				   .layerCount = 1
			},
			.imageExtent = inDstImage.mImageSize,
		};

		vk::CopyMemoryToImageInfo copyInfo
		{
			.dstImage = inDstImage.mVkImage,
			.dstImageLayout = inImageLayout,
			.regionCount = 1,
			.pRegions = &copy
		};

		mDevice.copyMemoryToImage(copyInfo);
	}

	mage::Array<cstr> Renderer::GetRequiredDeviceExtensions() const
	{
		mage::Array<cstr> result;

		result.Add(vk::KHRSwapchainExtensionName);
		result.Add(vk::KHRSwapchainMaintenance1ExtensionName);
		result.Add(vk::EXTShaderObjectExtensionName);
		result.Add(vk::EXTConservativeRasterizationExtensionName);

		return result;
	}

	u32 Renderer::SelectMemoryType(u32 inTypeFilter, vk::MemoryPropertyFlags inProperties) const
	{
		vk::PhysicalDeviceMemoryProperties memProperties = mPhysicalDevice.getMemoryProperties();
		for (u32 i = 0; i < memProperties.memoryTypeCount; ++i)
			if ((inTypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & inProperties) == inProperties)
				return i;

		mage_check(false);
		return u32(-1);
	}

	vk::raii::PhysicalDevice Renderer::PickPhysicalDevice(Instance const& inInstance) const
	{
		return inInstance.PickPhysicalDevice([this](vk::PhysicalDevice const& inDevice) -> u32
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
			});
	}

	void Renderer::SetupGraphicsQueue()
	{
		mage::Array<vk::QueueFamilyProperties> queueFamilies = mPhysicalDevice.getQueueFamilyProperties();
		u32 graphicsQueueIndex = u32(-1);

		for (u32 i = 0; i < queueFamilies.GetSize(); ++i)
		{
			vk::QueueFamilyProperties const& queueFamily = queueFamilies[i];

			if ((queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlags(0))
				continue;

			if (!mPhysicalDevice.getSurfaceSupportKHR(i, mSurface))
				continue;

			graphicsQueueIndex = i;
			break;
		}

		mage_check(graphicsQueueIndex != u32(-1));

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
			{ .features {
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

		mDevice = mPhysicalDevice.createDevice(deviceCreateInfo);
		mGraphicsQueue = mDevice.getQueue(graphicsQueueIndex, 0);

		vk::CommandPoolCreateInfo commandPoolCreateInfo
		{
			.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			.queueFamilyIndex = graphicsQueueIndex
		};

		mCommandPool = mDevice.createCommandPool(commandPoolCreateInfo);

		vk::CommandBufferAllocateInfo commandBufferAllocInfo
		{
			.commandPool = mCommandPool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = cMaxFramesInFlight
		};

		mCommandBuffers = mDevice.allocateCommandBuffers(commandBufferAllocInfo);

		for (u32 i = 0; i < cMaxFramesInFlight; ++i)
		{
			mPresentCompleteSemaphores.Add(mDevice.createSemaphore({}));
			mRenderFinishedSemaphores.Add(mDevice.createSemaphore({}));
			mDrawFences.Add(mDevice.createFence({ .flags = vk::FenceCreateFlagBits::eSignaled }));
			mPresentFences.Add(mDevice.createFence({ .flags = vk::FenceCreateFlagBits::eSignaled }));
		}
	}

	void Renderer::RecreateSwapchain()
	{
		mDevice.waitIdle();

		vk::SurfaceCapabilitiesKHR capabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);
		mage::Array<vk::SurfaceFormatKHR> formats = mPhysicalDevice.getSurfaceFormatsKHR(mSurface);
		mage::Array<vk::PresentModeKHR> presentModes = mPhysicalDevice.getSurfacePresentModesKHR(mSurface);

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
		mSwapchain = mDevice.createSwapchainKHR(swapchainCreateInfo);
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
			mSwapchainImageViews.Add(mDevice.createImageView(imageViewCreateInfo));
		}

		ImageCreateInfo colorImageCreateInfo
		{
			.Size = { mSwapchainExtent.width, mSwapchainExtent.height, 1 },
			.Format = mSwapchainSurfaceFormat.format,
			.AspectFlags = vk::ImageAspectFlagBits::eColor,
			.UsageFlags = vk::ImageUsageFlagBits::eColorAttachment,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
			.SampleCount = msaaSamples
		};

		mColorImage = CreateImage(colorImageCreateInfo);

		ImageCreateInfo depthImageCreateInfo
		{
			.Size = { mSwapchainExtent.width, mSwapchainExtent.height, 1 },
			.Format = vk::Format::eD32Sfloat,
			.AspectFlags = vk::ImageAspectFlagBits::eDepth,
			.UsageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
			.SampleCount = msaaSamples
		};

		mDepthImage = CreateImage(depthImageCreateInfo);
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
