#include "Engine/AssetManager.h"

AssetManager::AssetList::~AssetList()
{
	for (auto& asset : mAssets)
		delete asset.second;
}

bool AssetManager::AssetList::Register(Asset* inAsset, mage::StringView inName)
{
	if (mAssets.contains(inName)) return false;

	mAssets[inName] = inAsset;
	return true;
}

Asset* AssetManager::AssetList::Get(mage::StringView inName) const
{
	return mAssets.contains(inName) ? mAssets.at(inName) : nullptr;
}

Asset const* AssetManager::Get(std::type_index inType, mage::StringView inName) const
{
	if (mAssetLists.contains(inType))
		return mAssetLists.at(inType).Get(inName);

	return nullptr;
}
