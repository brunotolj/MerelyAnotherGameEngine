#include "Assets/Texture.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Renderer.h"

vk::DescriptorImageInfo Texture::GetDescriptorInfo() const
{
	vk::DescriptorImageInfo result = mImage.GetDescriptorInfo();
	result.sampler = mSampler;
	return result;
}

void Texture::CreateImage(Vulkan::Renderer const& inRenderer)
{
	vk::DeviceSize dataSize = mSize.width * mSize.height * mSize.depth * 4;

	Vulkan::BufferCreateInfo stagingBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	Vulkan::ImageCreateInfo imageCreateInfo
	{
		.Size = mSize,
		.Format = vk::Format::eR8G8B8A8Srgb,
		.AspectFlags = vk::ImageAspectFlagBits::eColor,
		.UsageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eHostTransfer,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
		.SampleCount = vk::SampleCountFlagBits::e1
	};

	mImage = inRenderer.CreateImage(imageCreateInfo);
	inRenderer.CopyMemoryToImage(mData.GetData(), mImage, vk::ImageLayout::eShaderReadOnlyOptimal);

	vk::SamplerCreateInfo samplerCreateInfo
	{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.anisotropyEnable = vk::True,
		.compareEnable = vk::False,
		.compareOp = vk::CompareOp::eAlways
	};

	mSampler = inRenderer.CreateImageSampler(samplerCreateInfo);
}
