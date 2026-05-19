#pragma once

#include "Assets/Texture.h"

namespace Vulkan
{
	class Renderer;
}

template<>
class Factory<Texture>
{
public:
	static AssetHandle<Texture> FromFile(mage::StringView inPath, Vulkan::Renderer const& inRenderer);

private:
	Factory() {}
};
