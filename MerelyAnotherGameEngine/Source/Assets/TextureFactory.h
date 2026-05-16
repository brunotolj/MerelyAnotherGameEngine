#pragma once

#include "Assets/AssetManager.h"
#include "Assets/Texture.h"

namespace Vulkan
{
	class Renderer;
}

template<>
class Factory<Texture>
{
public:
	static AssetHandle<Texture> FromFile(mage::StringView inPath, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager);

private:
	Factory() {}
};
