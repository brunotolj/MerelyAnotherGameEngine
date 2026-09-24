#pragma once

#include "Assets/Asset.h"

struct WorldComponentSetup
{
	mage::String Type;
	PropertyContainer Properties;
};

struct EntitySetup
{
	mage::String Type;
	PropertyContainer Properties;
	u32 ParentChainDepth = 0;
};

class WorldSetup : public Asset
{
	friend class Factory<WorldSetup>;

public:
	mage::Array<WorldComponentSetup> mComponentSetups;

	mage::Array<EntitySetup> mEntitySetups;

private:
	WorldSetup() {}

	mage::Array<AssetHandleBase> mAssetHandles;
};
