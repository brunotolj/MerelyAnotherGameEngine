#pragma once

#include "Assets/WorldSetup.h"

template<>
class Factory<WorldSetup>
{
public:
	static AssetHandle<WorldSetup> FromFile(mage::StringView inPath);

private:
	Factory() {}
};
