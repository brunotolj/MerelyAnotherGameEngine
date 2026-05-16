#include "Assets/AssetManager.h"

AssetManager::AssetList::~AssetList()
{
	for (auto& asset : mAssets)
		delete asset.second;
}

u32 AssetManager::AssetList::Register(Asset* inAsset)
{
	mAssets[++mIdCounter] = inAsset;
	return mIdCounter;
}

Asset* AssetManager::AssetList::Get(u32 inAssetId) const
{
	return mAssets.contains(inAssetId) ? mAssets.at(inAssetId) : nullptr;
}

Asset const* AssetManager::Get(std::type_index inType, u32 inAssetId) const
{
	if (mAssetLists.contains(inType))
		return mAssetLists.at(inType).Get(inAssetId);

	return nullptr;
}
