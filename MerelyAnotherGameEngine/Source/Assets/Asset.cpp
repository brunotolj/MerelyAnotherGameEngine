#include "Assets/Asset.h"
#include "Assets/AssetManager.h"

Asset const* AssetHandleBase::GetAsset(std::type_index inType) const
{
    return mAssetManager ? mAssetManager->Get(inType, mAssetId) : nullptr;
}
