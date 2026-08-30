#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	class Buffer;

	class Image : public NonMovable
	{
	public:
		struct CreateInfo
		{
			vk::Extent3D Size;
			vk::Format Format;
			vk::ImageAspectFlags AspectFlags;
			vk::ImageUsageFlags UsageFlags;
			vk::MemoryPropertyFlags MemoryFlags;
			vk::SampleCountFlagBits SampleCount;
		};

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
		Image(CreateInfo const& inCreateInfo);

		void Create(CreateInfo const& inCreateInfo);

		vk::raii::ImageView const& GetVkImageView() const;
		vk::ImageLayout GetLayout() const;

		void CopyFromMemory(void* inSrcMemory, vk::ImageLayout inImageLayout);
		void CopyFromBuffer(vk::CommandBuffer inCommandBuffer, Buffer const& inSrcBuffer) const;

		void TransitionLayout(vk::CommandBuffer inCommandBuffer, TransitionLayoutParams const& inParams);

		static void TransitionLayout(vk::CommandBuffer inCommandBuffer, vk::Image inImage, TransitionLayoutParams const& inParams, vk::ImageAspectFlags inAspectMask);

	private:
		vk::raii::Image mVkImage = nullptr;
		vk::raii::DeviceMemory mDeviceMemory = nullptr;
		vk::raii::ImageView mImageView = nullptr;

		vk::Extent3D mImageSize;
		vk::ImageLayout mImageLayout;
		vk::ImageAspectFlags mAspectMask;
	};
}
