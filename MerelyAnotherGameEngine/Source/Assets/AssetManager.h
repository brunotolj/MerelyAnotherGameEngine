#pragma once

#include "Assets/Asset.h"

#include <map>

class AssetManager : public NonMovableClass
{
	template <typename Type>
	friend class Factory;

public:
	Asset const* Get(std::type_index inType, u32 inAssetId) const;

private:
	class AssetList
	{
	public:
		~AssetList();

		u32 Register(Asset* inAsset);

		Asset* Get(u32 inAssetId) const;

	private:
		std::map<u32, Asset*> mAssets;
		u32 mIdCounter = 0;
	};

	template <AssetType Type>
	AssetHandle<Type> Register(Type* inAsset)
	{
		if (!mage_ensure(inAsset))
			return AssetHandle<Type>(*this, 0);

		return AssetHandle<Type>(*this, mAssetLists[typeid(Type)].Register(inAsset));
	}

	std::map<std::type_index, AssetList> mAssetLists;
};
