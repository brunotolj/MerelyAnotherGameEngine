#include "Vulkan/Texture.h"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

namespace Vulkan
{
	Texture& Texture::operator=(Texture&& inTexture)
	{
		mImage = std::move(inTexture.mImage);
		mDeviceMemory = std::move(inTexture.mDeviceMemory);
		mImageView = std::move(inTexture.mImageView);
		mSampler = std::move(inTexture.mSampler);

		return *this;
	}

	vk::DescriptorImageInfo Texture::GetDescriptorInfo() const
	{
		return vk::DescriptorImageInfo
		{
			.sampler = mSampler,
			.imageView = mImageView,
			.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
		};
	}

	TextureImageData Texture::LoadImage(mage::StringView inPath)
	{
		TextureImageData result;
		i32 bytesPerPixel;

		stbi_uc* imageData = stbi_load(inPath.GetCString(), reinterpret_cast<i32*>(&result.Size.width), reinterpret_cast<i32*>(&result.Size.height), &bytesPerPixel, 4);
		u32 imageSize = result.Size.width * result.Size.height * 4;

		result.Data.ResizeUninitialized(imageSize);
		memcpy(result.Data.GetData(), imageData, imageSize);

		result.Size.depth = 1;

		stbi_image_free(imageData);

		return result;
	}
}
