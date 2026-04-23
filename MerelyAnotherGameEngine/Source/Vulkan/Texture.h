#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	struct TextureImageData
	{
		mage::Array<u8> Data;
		vk::Extent3D Size;
	};

	struct TextureCreateInfo
	{
		TextureImageData ImageData;
	};

	class Texture : public NonCopyableClass
	{
		friend class Renderer;

	public:
		Texture(Texture&& inTexture) { *this = std::move(inTexture); };
		Texture& operator=(Texture&& inTexture);

		vk::DescriptorImageInfo GetDescriptorInfo() const;

		static TextureImageData LoadImage(mage::StringView inPath);

	private:
		Texture() {}

		vk::raii::Image mImage = nullptr;
		vk::raii::DeviceMemory mDeviceMemory = nullptr;
		vk::raii::ImageView mImageView = nullptr;
		vk::raii::Sampler mSampler = nullptr;
	};
}
