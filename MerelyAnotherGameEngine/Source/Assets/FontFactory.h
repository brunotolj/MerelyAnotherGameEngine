#pragma once

#include "Assets/Font.h"

template<>
class Factory<Font>
{
public:
	static AssetHandle<Font> FromFile(mage::StringView inPath);

private:
	Factory() {}
};
