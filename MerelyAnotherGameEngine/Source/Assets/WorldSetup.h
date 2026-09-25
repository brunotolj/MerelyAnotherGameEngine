#pragma once

#include "Assets/Asset.h"

struct WorldComponentSetup
{
	mage::String Type;
	PropertyValueMap Properties;
};

struct EntitySetup
{
	mage::String Type;
	PropertyValueMap Properties;
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

template<>
class Factory<WorldSetup>
{
public:
	static AssetHandle<WorldSetup> FromFile(mage::StringView inPath);

private:
	Factory() {}
};
