#pragma once

#include "Vulkan/Image.h"

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	class Renderer;

	class Texture : public NonMovableClass
	{
	public:
		struct CreateInfo
		{
			mage::Array<u8> Data;
			vk::Extent3D Size;
		};

		Texture(Renderer const& inRenderer, CreateInfo const& inCreateInfo);

		vk::DescriptorImageInfo GetDescriptorInfo() const;

		static CreateInfo LoadFromFile(mage::StringView inPath);

	private:
		void CreateImage(Renderer const& inRenderer, CreateInfo const& inData);

		Image mImage = nullptr;

		vk::raii::Sampler mSampler = nullptr;
	};
}
