#include "Vulkan/Image.h"
#include "Engine/Engine.h"
#include "Vulkan/Buffer.h"

namespace Vulkan
{
	Image::Image(Image::CreateInfo const& inCreateInfo)
	{
		Create(inCreateInfo);
	}

	void Image::Create(CreateInfo const& inCreateInfo)
	{
		vk::raii::Device const& device = gEngine->mVulkanDevice.GetVkDevice();

		vk::ImageCreateInfo imageCreateInfo
		{
			.imageType = vk::ImageType::e2D,
			.format = inCreateInfo.Format,
			.extent = inCreateInfo.Size,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = inCreateInfo.SampleCount,
			.tiling = vk::ImageTiling::eOptimal,
			.usage = inCreateInfo.UsageFlags,
			.sharingMode = vk::SharingMode::eExclusive
		};

		mVkImage = device.createImage(imageCreateInfo);
		mImageSize = inCreateInfo.Size;
		mAspectMask = inCreateInfo.AspectFlags;

		vk::MemoryRequirements memRequirements = mVkImage.getMemoryRequirements();
		u32 memoryTypeIndex = gEngine->mVulkanDevice.SelectMemoryType(memRequirements.memoryTypeBits, inCreateInfo.MemoryFlags);

		vk::MemoryAllocateInfo memoryAllocInfo
		{
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = memoryTypeIndex
		};

		mDeviceMemory = device.allocateMemory(memoryAllocInfo);
		mVkImage.bindMemory(mDeviceMemory, 0);

		vk::ImageViewCreateInfo imageViewCreateInfo
		{
			.image = mVkImage,
			.viewType = vk::ImageViewType::e2D,
			.format = inCreateInfo.Format,
			.subresourceRange
			{
				.aspectMask = inCreateInfo.AspectFlags,
				.levelCount = 1,
				.layerCount = 1
			}
		};

		mImageView = device.createImageView(imageViewCreateInfo);
	}

	vk::raii::ImageView const& Image::GetVkImageView() const
	{
		return mImageView;
	}

	vk::ImageLayout Image::GetLayout() const
	{
		return mImageLayout;
	}

	void Image::CopyFromMemory(void* inSrcMemory, vk::ImageLayout inImageLayout)
	{
		vk::raii::Device const& device = gEngine->mVulkanDevice.GetVkDevice();

		vk::HostImageLayoutTransitionInfo layoutTransitionInfo
		{
			.image = mVkImage,
			.oldLayout = vk::ImageLayout::eUndefined,
			.newLayout = inImageLayout,
			.subresourceRange
			{
				   .aspectMask = mAspectMask,
				   .baseMipLevel = 0,
				   .levelCount = 1,
				   .baseArrayLayer = 0,
				   .layerCount = 1
			}
		};

		device.transitionImageLayout(layoutTransitionInfo);
		mImageLayout = inImageLayout;

		vk::MemoryToImageCopy copy
		{
			.pHostPointer = inSrcMemory,
			.imageSubresource
			{
				   .aspectMask = mAspectMask,
				   .mipLevel = 0,
				   .baseArrayLayer = 0,
				   .layerCount = 1
			},
			.imageExtent = mImageSize,
		};

		vk::CopyMemoryToImageInfo copyInfo
		{
			.dstImage = mVkImage,
			.dstImageLayout = inImageLayout,
			.regionCount = 1,
			.pRegions = &copy
		};

		device.copyMemoryToImage(copyInfo);
	}

	void Image::CopyFromBuffer(vk::CommandBuffer inCommandBuffer, Buffer const& inSrcBuffer) const
	{
		vk::BufferImageCopy copyRegion
		{
			.imageSubresource = { .aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1 },
			.imageExtent = mImageSize
		};

		inCommandBuffer.copyBufferToImage(inSrcBuffer.GetVkBuffer(), mVkImage, vk::ImageLayout::eTransferDstOptimal, {copyRegion});
	}

	void Image::TransitionLayout(vk::CommandBuffer inCommandBuffer, TransitionLayoutParams const& inParams)
	{
		TransitionLayout(inCommandBuffer, mVkImage, inParams, mAspectMask);
		mImageLayout = inParams.NewLayout;
	}

	void Image::TransitionLayout(vk::CommandBuffer inCommandBuffer, vk::Image inImage, TransitionLayoutParams const& inParams, vk::ImageAspectFlags inAspectMask)
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
				   .aspectMask = inAspectMask,
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
}
