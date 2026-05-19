#pragma once

#include "Assets/Font.h"

namespace Vulkan
{
	class Renderer;
}

template<>
class Factory<Font>
{
public:
	static AssetHandle<Font> FromFile(mage::StringView inPath, Vulkan::Renderer const& inRenderer);

private:
	Factory() {}
};
