#pragma once

#include "Assets/Texture.h"

template<>
class Factory<Texture>
{
public:
	static AssetHandle<Texture> FromFile(mage::StringView inPath);

private:
	Factory() {}
};
