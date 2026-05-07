#include "Vulkan/Texture.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Renderer.h"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

namespace Vulkan
{
	Texture::Texture(Renderer const& inRenderer, Texture::CreateInfo const& inCreateInfo)
	{
		CreateImage(inRenderer, inCreateInfo);
	}

	vk::DescriptorImageInfo Texture::GetDescriptorInfo() const
	{
		vk::DescriptorImageInfo result = mImage.GetDescriptorInfo();
		result.sampler = mSampler;
		return result;
	}

	Texture::CreateInfo Texture::LoadFromFile(mage::StringView inPath)
	{
		Texture::CreateInfo result;
		i32 bytesPerPixel;

		stbi_uc* imageData = stbi_load(inPath.GetCString(), reinterpret_cast<i32*>(&result.Size.width), reinterpret_cast<i32*>(&result.Size.height), &bytesPerPixel, 4);
		u32 imageSize = result.Size.width * result.Size.height * 4;

		result.Data.ResizeUninitialized(imageSize);
		memcpy(result.Data.GetData(), imageData, imageSize);

		result.Size.depth = 1;

		stbi_image_free(imageData);

		return result;
	}

	void Texture::CreateImage(Renderer const& inRenderer, Texture::CreateInfo const& inData)
	{
		vk::DeviceSize dataSize = inData.Size.width * inData.Size.height * 4;

		BufferCreateInfo stagingBufferCreateInfo
		{
			.Size = dataSize,
			.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		};

		ImageCreateInfo imageCreateInfo
		{
			.Size = inData.Size,
			.Format = vk::Format::eR8G8B8A8Srgb,
			.AspectFlags = vk::ImageAspectFlagBits::eColor,
			.UsageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eHostTransfer,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
			.SampleCount = vk::SampleCountFlagBits::e1
		};

		mImage = inRenderer.CreateImage(imageCreateInfo);
		inRenderer.CopyMemoryToImage(inData.Data.GetData(), mImage, vk::ImageLayout::eShaderReadOnlyOptimal);

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
}
