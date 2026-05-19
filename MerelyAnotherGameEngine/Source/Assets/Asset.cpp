#include "Assets/Asset.h"
#include "Engine/Engine.h"

Asset const* AssetHandleBase::GetAsset(std::type_index inType) const
{
    return gEngine->mAssetManager.Get(inType, mAssetId);
}
