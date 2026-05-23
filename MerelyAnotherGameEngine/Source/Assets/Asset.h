#pragma once

#include <typeindex>

class Asset : public NonMovableClass
{
public:
	virtual ~Asset() {};
};

template <typename Type>
concept AssetType = std::derived_from<Type, Asset> && !std::same_as<Type, Asset>;

template <typename Type>
class Factory;

class AssetHandleBase
{
public:
	Asset const* GetAsset(std::type_index inType) const;

protected:
	AssetHandleBase(u32 inAssetId) : mAssetId(inAssetId) {}

	u32 mAssetId;
};

template <AssetType Type>
class AssetHandle : public AssetHandleBase
{
	friend class AssetManager;

public:
	AssetHandle() : AssetHandleBase(0) {}

	Type const* GetAsset() const
	{
		return (Type const*)(AssetHandleBase::GetAsset(typeid(Type)));
	}

private:
	AssetHandle(u32 inAssetId) : AssetHandleBase(inAssetId) {}
};
