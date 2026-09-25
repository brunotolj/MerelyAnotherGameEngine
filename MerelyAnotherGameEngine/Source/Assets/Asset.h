#pragma once

#include <typeindex>

class Asset : public NonMovable
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

	bool IsSet() { return mName.GetLength() > 0; }

protected:
	AssetHandleBase(mage::String const& inName) : mName(inName) {}

	mage::String mName;
};

template <AssetType Type>
class AssetHandle : public AssetHandleBase
{
	friend class AssetManager;

public:
	AssetHandle(nullptr_t) : AssetHandleBase("") {}

	Type const* GetAsset() const
	{
		return (Type const*)(AssetHandleBase::GetAsset(typeid(Type)));
	}

private:
	AssetHandle(mage::String const& inName) : AssetHandleBase(inName) {}
};

using PropertyValueMap = std::unordered_map<mage::String, mage::String>;
using AssetFactoryCallback = std::function<AssetHandleBase(mage::StringView, PropertyValueMap const&)>;

extern std::unordered_map<mage::String, AssetFactoryCallback> gAssetFactoryFunctions;

class AssetFactoryFunction
{
public:
	AssetFactoryFunction(mage::StringView inName, AssetFactoryCallback&& inFunction)
	{
		gAssetFactoryFunctions[inName] = inFunction;
	}
};

#define REGISTER_ASSET_FACTORY_FUNCTION(Type) \
AssetFactoryFunction Type##FactoryFunction(#Type, [](mage::StringView inName, PropertyValueMap const& inProperties) \
{ \
	Factory<Type>::Impl factory; \
	PropertyTree(factory).ApplyPropertyValues(inProperties); \
	return factory.Create(inName); \
});

#define REGISTER_ASSET_FACTORY_FUNCTION_WITH_SUBTYPE(Type, Subtype) \
AssetFactoryFunction Type##Subtype##FactoryFunction(#Type ":" #Subtype, [](mage::StringView inName, PropertyValueMap const& inProperties) \
{ \
	Factory<Type>::Subtype factory; \
	PropertyTree(factory).ApplyPropertyValues(inProperties); \
	return factory.Create(inName); \
});
