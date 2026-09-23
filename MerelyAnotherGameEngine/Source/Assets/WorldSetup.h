#pragma once

#include "Assets/Asset.h"

struct WorldComponentSetup
{
	mage::String Name;
	PropertyContainer Properties;
};

class WorldSetup : public Asset
{
	friend class Factory<WorldSetup>;

public:
	mage::Array<WorldComponentSetup> mComponentSetups;

private:
	WorldSetup() {}

	mage::Array<AssetHandleBase> mAssetHandles;
};
