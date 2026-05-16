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

class AssetManager;

class AssetHandleBase
{
public:
	Asset const* GetAsset(std::type_index inType) const;

protected:
	AssetHandleBase(AssetManager const* inAssetManager, u32 inAssetId)
		: mAssetManager(inAssetManager), mAssetId(inAssetId) {}

	AssetManager const* mAssetManager;
	u32 mAssetId;
};

template <AssetType Type>
class AssetHandle : public AssetHandleBase
{
	friend AssetManager;

public:
	AssetHandle() : AssetHandleBase(nullptr, 0) {}

	Type const* GetAsset() const
	{
		return (Type const*)(AssetHandleBase::GetAsset(typeid(Type)));
	}

private:
	AssetHandle(AssetManager const& inAssetManager, u32 inAssetId)
		: AssetHandleBase(&inAssetManager, inAssetId) {}
};
