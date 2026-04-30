#include "Vulkan/Image.h"
#include "Vulkan/Buffer.h"

namespace Vulkan
{
    Image& Image::operator=(Image&& inImage)
    {
		mVkImage = std::move(inImage.mVkImage);
		mDeviceMemory = std::move(inImage.mDeviceMemory);
		mImageView = std::move(inImage.mImageView);
		std::swap(mImageSize, inImage.mImageSize);
		std::swap(mImageLayout, inImage.mImageLayout);
		std::swap(mAspectMask, inImage.mAspectMask);

		return *this;
    }

	vk::DescriptorImageInfo Image::GetDescriptorInfo() const
	{
		return vk::DescriptorImageInfo
		{
			.imageView = mImageView,
			.imageLayout = mImageLayout
		};
	}

	void Image::CopyFromBuffer(vk::CommandBuffer inCommandBuffer, Buffer const& inSrcBuffer) const
	{
		vk::BufferImageCopy copyRegion
		{
			.imageSubresource = { .aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1 },
			.imageExtent = mImageSize
		};

		inCommandBuffer.copyBufferToImage(inSrcBuffer.mVkBuffer, mVkImage, vk::ImageLayout::eTransferDstOptimal, { copyRegion });
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
