#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	class Buffer;

	struct ImageCreateInfo
	{
		vk::Extent3D Size;
		vk::Format Format;
		vk::ImageAspectFlags AspectFlags;
		vk::ImageUsageFlags UsageFlags;
		vk::MemoryPropertyFlags MemoryFlags;
		vk::SampleCountFlagBits SampleCount;
	};

	class Image : public NonCopyableClass
	{
		friend class Renderer;

	public:
		struct TransitionLayoutParams
		{
			vk::PipelineStageFlags2 SrcStageMask;
			vk::AccessFlags2 SrcAccessMask;
			vk::PipelineStageFlags2 DstStageMask;
			vk::AccessFlags2 DstAccessMask;
			vk::ImageLayout OldLayout;
			vk::ImageLayout NewLayout;
		};

		Image(nullptr_t) {}
		Image(Image&& inImage) { *this = std::move(inImage); };
		Image& operator=(Image&& inImage);

		vk::DescriptorImageInfo GetDescriptorInfo() const;

		void CopyFromBuffer(vk::CommandBuffer inCommandBuffer, Buffer const& inSrcBuffer) const;
		void TransitionLayout(vk::CommandBuffer inCommandBuffer, TransitionLayoutParams const& inParams);

	private:
		static void TransitionLayout(vk::CommandBuffer inCommandBuffer, vk::Image inImage, TransitionLayoutParams const& inParams, vk::ImageAspectFlags inAspectMask);

		vk::raii::Image mVkImage = nullptr;
		vk::raii::DeviceMemory mDeviceMemory = nullptr;
		vk::raii::ImageView mImageView = nullptr;

		vk::Extent3D mImageSize;
		vk::ImageLayout mImageLayout;
		vk::ImageAspectFlags mAspectMask;
	};
}
