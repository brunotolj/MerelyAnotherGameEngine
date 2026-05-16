#pragma once

#include "Assets/Asset.h"
#include "Vulkan/Image.h"

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	class Renderer;
}

class Texture : public Asset
{
	friend class Factory<Texture>;

public:
	vk::DescriptorImageInfo GetDescriptorInfo() const;

private:
	Texture() {}

	void CreateImage(Vulkan::Renderer const& inRenderer);

	Vulkan::Image mImage = nullptr;
	vk::raii::Sampler mSampler = nullptr;

	mage::Array<u8> mData;
	vk::Extent3D mSize;
};
