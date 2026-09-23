#pragma once

#include "Assets/Asset.h"

class WorldSetup : public Asset
{
	friend class Factory<WorldSetup>;

private:
	WorldSetup() {}

	mage::Array<AssetHandleBase> mAssetHandles;
};
