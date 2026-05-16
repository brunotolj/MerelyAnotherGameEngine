#pragma once

#include "Assets/AssetManager.h"
#include "Assets/Font.h"

namespace Vulkan
{
	class Renderer;
}

template<>
class Factory<Font>
{
public:
	static AssetHandle<Font> FromFile(mage::StringView inPath, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager);

private:
	Factory() {}
};
