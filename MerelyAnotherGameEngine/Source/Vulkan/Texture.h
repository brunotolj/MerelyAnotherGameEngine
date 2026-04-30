#pragma once

#include "Vulkan/Image.h"

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	class Renderer;

	struct TextureCreateInfo
	{
		mage::Array<u8> Data;
		vk::Extent3D Size;
	};

	class Texture : public NonMovableClass
	{
	public:
		Texture(Renderer const& inRenderer, TextureCreateInfo const& inCreateInfo);

		vk::DescriptorImageInfo GetDescriptorInfo() const;

		static TextureCreateInfo LoadFromFile(mage::StringView inPath);

	private:
		void CreateImage(Renderer const& inRenderer, TextureCreateInfo const& inData);

		Image mImage = nullptr;

		vk::raii::Sampler mSampler = nullptr;
	};
}
