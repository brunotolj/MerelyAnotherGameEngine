#include "Vulkan/Renderer.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Texture.h"
#include "Vulkan/VulkanInterface.h"
#include "Vulkan/Window.h"

namespace Vulkan
{
	Vulkan::Renderer::Renderer(Instance const& inInstance, Window& inWindow)
	{
		mPhysicalDevice = PickPhysicalDevice(inInstance);
		mage_check(mPhysicalDevice != nullptr);

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
			TransitionImageLayoutParams transitionParams
			{
				.SrcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				.SrcAccessMask = {},
				.DstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				.DstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
				.OldLayout = vk::ImageLayout::eUndefined,
				.NewLayout = vk::ImageLayout::eColorAttachmentOptimal,
				.AspectMask = vk::ImageAspectFlagBits::eColor
			};

			TransitionImageLayout(commandBuffer, currentImage, transitionParams);
		}

		{
			TransitionImageLayoutParams transitionParams
			{
				.SrcStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				.SrcAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				.DstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				.DstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				.OldLayout = vk::ImageLayout::eUndefined,
				.NewLayout = vk::ImageLayout::eDepthAttachmentOptimal,
				.AspectMask = vk::ImageAspectFlagBits::eDepth
			};

			TransitionImageLayout(commandBuffer, mDepthImage, transitionParams);
		}

		vk::RenderingAttachmentInfo colorAttachmentInfo
		{
			.imageView = currentImageView,
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = vk::ClearColorValue(0.1f, 0.1f, 0.15f, 1.0f)
		};

		vk::RenderingAttachmentInfo depthAttachmentInfo
		{
			.imageView = mDepthImageView,
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

		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, f32(mSwapchainExtent.width), f32(mSwapchainExtent.height), 0.0f, 1.0f));
		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), mSwapchainExtent));

		inFunction({ commandBuffer, mSwapchainExtent, mCurrentFrameIndex });

		commandBuffer.endRendering();

		{
			TransitionImageLayoutParams transitionParams
			{
				.SrcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				.SrcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
				.DstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
				.DstAccessMask = {},
				.OldLayout = vk::ImageLayout::eColorAttachmentOptimal,
				.NewLayout = vk::ImageLayout::ePresentSrcKHR,
				.AspectMask = vk::ImageAspectFlagBits::eColor
			};

			TransitionImageLayout(commandBuffer, currentImage, transitionParams);
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

		mage_check(inPipelineCreateInfo.VertexShaderCode.GetSize());
		mage_check(inPipelineCreateInfo.FragmentShaderCode.GetSize());

		for (DescriptorSetLayoutInfo const& layout : inPipelineCreateInfo.DescriptorSetLayouts)
		{
			vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
			{
				.bindingCount = layout.Bindings.GetSize(),
				.pBindings = layout.Bindings.GetData()
			};

			result.mDescriptorSetLayouts.Add(mDevice.createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
		}

		u32 descriptorSetCount = 0;
		for (vk::DescriptorPoolSize const& poolSize : inPipelineCreateInfo.DescriptorPoolSizes)
			descriptorSetCount += poolSize.descriptorCount;

		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo
		{
			.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			.maxSets = descriptorSetCount,
			.poolSizeCount = inPipelineCreateInfo.DescriptorPoolSizes.GetSize(),
			.pPoolSizes = inPipelineCreateInfo.DescriptorPoolSizes.GetData()
		};

		result.mDescriptorPool = mDevice.createDescriptorPool(descriptorPoolCreateInfo);

		{
			mage::Array<vk::DescriptorSetLayout> layouts;
			layouts.Reserve(descriptorSetCount);

			for (u32 i = 0; i < inPipelineCreateInfo.DescriptorSetLayouts.GetSize(); ++i)
				for (u32 j = 0; j < inPipelineCreateInfo.DescriptorSetLayouts[i].Count; ++j)
					layouts.Add(result.mDescriptorSetLayouts[i]);

			vk::DescriptorSetAllocateInfo descriptorSetAllocInfo
			{
				.descriptorPool = result.mDescriptorPool,
				.descriptorSetCount = layouts.GetSize(),
				.pSetLayouts = layouts.GetData()
			};

			result.mDescriptorSets = mDevice.allocateDescriptorSets(descriptorSetAllocInfo);
		}

		{
			mage::Array<vk::DescriptorSetLayout> layouts(result.mDescriptorSetLayouts.GetSize());
			for (u32 i = 0; i < layouts.GetSize(); ++i)
				layouts[i] = result.mDescriptorSetLayouts[i];

			vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo
			{
				.setLayoutCount = layouts.GetSize(),
				.pSetLayouts = layouts.GetData(),
				.pushConstantRangeCount = inPipelineCreateInfo.PushConstantRanges.GetSize(),
				.pPushConstantRanges = inPipelineCreateInfo.PushConstantRanges.GetData()
			};

			result.mPipelineLayout = mDevice.createPipelineLayout(pipelineLayoutCreateInfo);
		}

		mage::Array<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos;

		vk::ShaderModuleCreateInfo vertShaderModuleCreateInfo
		{
			.codeSize = inPipelineCreateInfo.VertexShaderCode.GetSize() * sizeof(u32),
			.pCode = inPipelineCreateInfo.VertexShaderCode.GetData()
		};

		vk::raii::ShaderModule vertShaderModule{ mDevice, vertShaderModuleCreateInfo };
		shaderStageCreateInfos.Add(vk::PipelineShaderStageCreateInfo
			{
				.stage = vk::ShaderStageFlagBits::eVertex,
				.module = vertShaderModule,
				.pName = "main"
			});

		vk::ShaderModuleCreateInfo fragShaderModuleCreateInfo
		{
			.codeSize = inPipelineCreateInfo.FragmentShaderCode.GetSize() * sizeof(u32),
			.pCode = inPipelineCreateInfo.FragmentShaderCode.GetData()
		};

		vk::raii::ShaderModule fragShaderModule{ mDevice, fragShaderModuleCreateInfo };
		shaderStageCreateInfos.Add(vk::PipelineShaderStageCreateInfo
			{
				.stage = vk::ShaderStageFlagBits::eFragment,
				.module = fragShaderModule,
				.pName = "main"
			});

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo
		{
			.vertexBindingDescriptionCount = inPipelineCreateInfo.BindingDescriptions.GetSize(),
			.pVertexBindingDescriptions = inPipelineCreateInfo.BindingDescriptions.GetData(),
			.vertexAttributeDescriptionCount = inPipelineCreateInfo.AttributeDescriptions.GetSize(),
			.pVertexAttributeDescriptions = inPipelineCreateInfo.AttributeDescriptions.GetData()
		};

		vk::PipelineViewportStateCreateInfo viewportInfo
		{
			.viewportCount = 1,
			.scissorCount = 1
		};

		vk::PipelineRasterizationStateCreateInfo rasterizationInfo
		{
			.depthClampEnable = vk::False,
			.rasterizerDiscardEnable = vk::False,
			.polygonMode = vk::PolygonMode::eFill,
			.cullMode = vk::CullModeFlagBits::eBack,
			.frontFace = vk::FrontFace::eClockwise,
			.depthBiasEnable = vk::False,
			.lineWidth = 1.0f
		};

		vk::PipelineMultisampleStateCreateInfo multisampleInfo
		{
			.rasterizationSamples = vk::SampleCountFlagBits::e1,
			.sampleShadingEnable = vk::False
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
			.depthTestEnable = vk::True,
			.depthWriteEnable = vk::True,
			.depthCompareOp = vk::CompareOp::eLess,
			.depthBoundsTestEnable = vk::False,
			.stencilTestEnable = vk::False
		};

		vk::PipelineColorBlendStateCreateInfo colorBlendInfo
		{
			.logicOpEnable = vk::False,
			.logicOp = vk::LogicOp::eCopy,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachmentState
		};

		mage::Array<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
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
				.pInputAssemblyState = &inPipelineCreateInfo.InputAssemblyInfo,
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
			.usage = inBufferCreateInfo.UsageFlags,
			.sharingMode = vk::SharingMode::eExclusive
		};

		result.mVkBuffer = mDevice.createBuffer(bufferCreateInfo);
		result.mBufferSize = inBufferCreateInfo.Size;

		vk::MemoryRequirements memRequirements = result.mVkBuffer.getMemoryRequirements();
		u32 memoryTypeIndex = SelectMemoryType(memRequirements.memoryTypeBits, inBufferCreateInfo.MemoryFlags);
			
		vk::MemoryAllocateInfo memoryAllocInfo
		{
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = memoryTypeIndex
		};

		result.mDeviceMemory = mDevice.allocateMemory(memoryAllocInfo);
		result.mVkBuffer.bindMemory(result.mDeviceMemory, 0);

		return result;
	}

	Texture Renderer::CreateTexture(TextureCreateInfo const& inTextureCreateInfo) const
	{
		Texture result;

		vk::Extent3D imageSize = inTextureCreateInfo.ImageData.Size;
		vk::DeviceSize dataSize = imageSize.width * imageSize.height * 4;

		Vulkan::BufferCreateInfo stagingBufferCreateInfo
		{
			.Size = dataSize,
			.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		};

		Vulkan::Buffer stagingBuffer = CreateBuffer(stagingBufferCreateInfo);
		stagingBuffer.Map();
		stagingBuffer.Write(inTextureCreateInfo.ImageData.Data.GetData(), dataSize);
		
		vk::ImageCreateInfo imageCreateInfo
		{
			.imageType = vk::ImageType::e2D,
			.format = vk::Format::eR8G8B8A8Srgb,
			.extent = imageSize,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = vk::SampleCountFlagBits::e1,
			.tiling = vk::ImageTiling::eOptimal,
			.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			.sharingMode = vk::SharingMode::eExclusive
		};

		result.mImage = mDevice.createImage(imageCreateInfo);

		vk::MemoryRequirements memRequirements = result.mImage.getMemoryRequirements();
		u32 memoryTypeIndex = SelectMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		vk::MemoryAllocateInfo memoryAllocInfo
		{
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = memoryTypeIndex
		};

		result.mDeviceMemory = mDevice.allocateMemory(memoryAllocInfo);
		result.mImage.bindMemory(result.mDeviceMemory, 0);

		SubmitSingleTimeCommands([this, &result, &stagingBuffer, imageSize](vk::CommandBuffer inCommandBuffer)
			{
				{
					TransitionImageLayoutParams transitionParams
					{
						.SrcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
						.SrcAccessMask = {},
						.DstStageMask = vk::PipelineStageFlagBits2::eTransfer,
						.DstAccessMask = vk::AccessFlagBits2::eTransferWrite,
						.OldLayout = vk::ImageLayout::eUndefined,
						.NewLayout = vk::ImageLayout::eTransferDstOptimal,
						.AspectMask = vk::ImageAspectFlagBits::eColor
					};

					TransitionImageLayout(inCommandBuffer, result.mImage, transitionParams);
				}

				CopyBufferToImage(inCommandBuffer, stagingBuffer, result.mImage, imageSize);

				{
					TransitionImageLayoutParams transitionParams
					{
						.SrcStageMask = vk::PipelineStageFlagBits2::eTransfer,
						.SrcAccessMask = vk::AccessFlagBits2::eTransferWrite,
						.DstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
						.DstAccessMask = vk::AccessFlagBits2::eShaderRead,
						.OldLayout = vk::ImageLayout::eTransferDstOptimal,
						.NewLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
						.AspectMask = vk::ImageAspectFlagBits::eColor
					};

					TransitionImageLayout(inCommandBuffer, result.mImage, transitionParams);
				}
			});

		vk::ImageViewCreateInfo imageViewCreateInfo
		{
			.image = result.mImage,
			.viewType = vk::ImageViewType::e2D,
			.format = vk::Format::eR8G8B8A8Srgb,
			.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
		};

		result.mImageView = mDevice.createImageView(imageViewCreateInfo);

		vk::PhysicalDeviceProperties properties = mPhysicalDevice.getProperties();
		
		vk::SamplerCreateInfo samplerCreateInfo
		{
			.magFilter = vk::Filter::eLinear,
			.minFilter = vk::Filter::eLinear,
			.mipmapMode = vk::SamplerMipmapMode::eLinear,
			.addressModeU = vk::SamplerAddressMode::eRepeat,
			.addressModeV = vk::SamplerAddressMode::eRepeat,
			.addressModeW = vk::SamplerAddressMode::eRepeat,
			.anisotropyEnable = vk::True,
			.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
			.compareEnable = vk::False,
			.compareOp = vk::CompareOp::eAlways
		};

		result.mSampler = mDevice.createSampler(samplerCreateInfo);

		return result;
	}

	void Renderer::CopyBuffer(vk::CommandBuffer inCommandBuffer, Buffer const& inSrcBuffer, Buffer const& inDstBuffer, vk::DeviceSize inSize) const
	{
		vk::BufferCopy copyRegion{ .size = inSize };
		inCommandBuffer.copyBuffer(*inSrcBuffer.mVkBuffer, *inDstBuffer.mVkBuffer, copyRegion);
	}

	void Renderer::CopyBufferToImage(vk::CommandBuffer inCommandBufer, Buffer const& inSrcBuffer, vk::Image inDstImage, vk::Extent3D inImageSize) const
	{
		vk::BufferImageCopy copyRegion
		{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
			.imageOffset = {0, 0, 0},
			.imageExtent = inImageSize
		};

		inCommandBufer.copyBufferToImage(inSrcBuffer.mVkBuffer, inDstImage, vk::ImageLayout::eTransferDstOptimal, { copyRegion });
	}

	void Renderer::TransitionImageLayout(vk::CommandBuffer inCommandBuffer, vk::Image inImage, TransitionImageLayoutParams inParams) const
	{
		vk::ImageMemoryBarrier2 barrier
		{
			.srcStageMask = inParams.SrcStageMask,
			.srcAccessMask = inParams.SrcAccessMask,
			.dstStageMask = inParams.DstStageMask,
			.dstAccessMask = inParams.DstAccessMask,
			.oldLayout = inParams.OldLayout,
			.newLayout = inParams.NewLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = inImage,
			.subresourceRange
			{
				   .aspectMask = inParams.AspectMask,
				   .baseMipLevel = 0,
				   .levelCount = 1,
				   .baseArrayLayer = 0,
				   .layerCount = 1
			}
		};

		vk::DependencyInfo dependencyInfo
		{
			.dependencyFlags = {},
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier
		};

		inCommandBuffer.pipelineBarrier2(dependencyInfo);
	}

	mage::Array<cstr> Renderer::GetRequiredDeviceExtensions() const
	{
		mage::Array<cstr> result;

		result.Add(vk::KHRSwapchainExtensionName);
		result.Add(vk::KHRSwapchainMaintenance1ExtensionName);

		return result;
	}

	u32 Renderer::SelectMemoryType(u32 typeFilter, vk::MemoryPropertyFlags properties) const
	{
		vk::PhysicalDeviceMemoryProperties memProperties = mPhysicalDevice.getMemoryProperties();
		for (u32 i = 0; i < memProperties.memoryTypeCount; ++i)
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
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
					vk::PhysicalDeviceVulkan13Features,
					vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
					vk::PhysicalDeviceSwapchainMaintenance1FeaturesKHR>();

				{
					auto& f = features.get<vk::PhysicalDeviceFeatures2>();
					if (!f.features.samplerAnisotropy) return 0;
				}

				{
					auto& f = features.get<vk::PhysicalDeviceVulkan13Features>();
					if (!f.synchronization2) return 0;
					if (!f.dynamicRendering) return 0;
				}

				{
					auto& f = features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
					if (!f.extendedDynamicState) return 0;
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
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
			vk::PhysicalDeviceSwapchainMaintenance1FeaturesKHR>;

		FeatureChain featureChain
		{
			{ .features {
				.samplerAnisotropy = true
			}},
			{
				.synchronization2 = true,
				.dynamicRendering = true
			},
			{
				.extendedDynamicState = true
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

		vk::ImageCreateInfo depthImageCreateInfo
		{
			.imageType = vk::ImageType::e2D,
			.format = vk::Format::eD32Sfloat,
			.extent = { mSwapchainExtent.width, mSwapchainExtent.height, 1 },
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = vk::SampleCountFlagBits::e1,
			.tiling = vk::ImageTiling::eOptimal,
			.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
			.sharingMode = vk::SharingMode::eExclusive
		};

		mDepthImage = mDevice.createImage(depthImageCreateInfo);

		vk::MemoryRequirements memRequirements = mDepthImage.getMemoryRequirements();
		u32 memoryTypeIndex = SelectMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		vk::MemoryAllocateInfo depthMemoryAllocInfo
		{
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = memoryTypeIndex
		};

		mDepthImageMemory = mDevice.allocateMemory(depthMemoryAllocInfo);
		mDepthImage.bindMemory(mDepthImageMemory, 0);

		vk::ImageViewCreateInfo depthImageViewCreateInfo
		{
			.image = mDepthImage,
			.viewType = vk::ImageViewType::e2D,
			.format = vk::Format::eD32Sfloat,
			.subresourceRange = { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }
		};

		mDepthImageView = mDevice.createImageView(depthImageViewCreateInfo);
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
		if (inCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
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
