#include "Assets/Texture.h"
#include "Engine/Engine.h"

vk::DescriptorImageInfo Texture::GetDescriptorInfo() const
{
	return vk::DescriptorImageInfo
	{
		.sampler = mSampler,
		.imageView = mImage.GetVkImageView(),
		.imageLayout = mImage.GetLayout()
	};
}

void Texture::CreateImage()
{
	vk::DeviceSize dataSize = mSize.width * mSize.height * mSize.depth * 4;

	Vulkan::Image::CreateInfo imageCreateInfo
	{
		.Size = mSize,
		.Format = vk::Format::eR8G8B8A8Srgb,
		.AspectFlags = vk::ImageAspectFlagBits::eColor,
		.UsageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eHostTransfer,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
		.SampleCount = vk::SampleCountFlagBits::e1
	};

	mImage.Create(imageCreateInfo);
	mImage.CopyFromMemory(mData.GetData(), vk::ImageLayout::eShaderReadOnlyOptimal);

	vk::SamplerCreateInfo samplerCreateInfo
	{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.anisotropyEnable = vk::True,
		.maxAnisotropy = gEngine->mVulkanDevice.GetVkPhysicalDevice().getProperties().limits.maxSamplerAnisotropy,
		.compareEnable = vk::False,
		.compareOp = vk::CompareOp::eAlways
	};

	mSampler = gEngine->mVulkanDevice.GetVkDevice().createSampler(samplerCreateInfo);
}
