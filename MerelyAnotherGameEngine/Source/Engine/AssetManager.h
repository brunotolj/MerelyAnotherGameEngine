#pragma once

#include "Assets/Asset.h"

#include <map>

class AssetManager : public NonMovable
{
	template <typename Type>
	friend class Factory;

public:
	Asset const* Get(std::type_index inType, mage::StringView inName) const;

	template <AssetType Type>
	AssetHandle<Type> GetHandle(mage::StringView inName)
	{
		if (mAssetLists[typeid(Type)].Get(inName))
			return AssetHandle<Type>(inName);

		return nullptr;
	}

private:
	class AssetList
	{
	public:
		~AssetList();

		bool Register(Asset* inAsset, mage::StringView inName);

		Asset* Get(mage::StringView inName) const;

	private:
		std::unordered_map<mage::String, Asset*> mAssets;
	};

	template <AssetType Type>
	AssetHandle<Type> Register(Type* inAsset, mage::StringView inName)
	{
		if (!mage_ensure(inAsset))
			return nullptr;

		if (mAssetLists[typeid(Type)].Register(inAsset, inName))
			return AssetHandle<Type>(inName);

		return nullptr;
	}

	std::map<std::type_index, AssetList> mAssetLists;
};
