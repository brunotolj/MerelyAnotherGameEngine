#pragma once

#include "Assets/WorldSetup.h"
#include "Engine/Engine.h"
#include "Framework/Entity.h"
#include "Framework/GameWorldComponent.h"

#include <typeindex>

class GameWorld : public NonCopyable
{
public:
	GameWorld(WorldSetup const& inWorldSetup);
	~GameWorld();

	void Update(f32 inDeltaTime);

	template <WorldComponentType Type>
	Type* CreateComponent(PropertyValueMap const& inProperties)
	{
		if (GetComponent<Type>()) return nullptr;
		if (Type::CheckPrerequisites(*this) == false) return nullptr;

		Type* component = new Type(*this);
		PropertyTree(*component).ApplyPropertyValues(inProperties);

		mage::Array<std::type_index> callbackTypes;
		component->GetEntityCallbackTypes(callbackTypes);
		for (std::type_index callbackType : callbackTypes)
			mEntityCallbacks[callbackType].Add(component);

		mComponentByClass[typeid(Type)] = component;

		if constexpr (std::is_base_of<GameUtility, Type>::value)
			mUtilities.Add(component);
		else if constexpr (std::is_base_of<GameSystem, Type>::value)
			mSystems.Add(component);
		else
			static_assert(false, "Component is neither a GameUtility or GameSystem");
		
		return component;
	}

	template <WorldComponentType Type>
	Type* GetComponent()
	{
		auto component = mComponentByClass.find(typeid(Type));
		if (component == mComponentByClass.end())
			return nullptr;

		return (Type*)(component->second);
	}

	template <EntityType Type, typename... Args>
	Type* CreateEntity(GameEntity* inParentEntity, Type::Setup const& inSetup)
	{
		if (Type::IsParentEntityValid(inParentEntity) == false)
			return nullptr;

		Type* entity = new Type(typeid(Type), inParentEntity, inSetup);

		mEntities.Add(entity);
		mEntitiesByClass[entity->mTypeIndex].Add(entity);

		if (mEntityCallbacks.contains(typeid(Type)))
			for (GameWorldComponent* component : mEntityCallbacks.at(typeid(Type)))
				component->OnEntityCreated(entity, &inSetup, typeid(Type));

		return entity;
	}

	mage::Array<GameEntity*> const& GetEntities() const { return mEntities; }

	template <EntityType Type>
	mage::Array<Type*> const& GetEntities() const
	{
		static mage::Array<Type*> dummy;
		if (mEntitiesByClass.contains(typeid(Type)) == false) return dummy;
		return (mage::Array<Type*> const&)(mEntitiesByClass.at(typeid(Type)));
	}

private:
	void DestroyEntity(GameEntity* inEntity);

	mage::Array<GameUtility*> mUtilities;
	mage::Array<GameSystem*> mSystems;
	std::unordered_map<std::type_index, GameWorldComponent*> mComponentByClass;

	mage::Array<GameEntity*> mEntities;
	std::unordered_map<std::type_index, mage::Array<GameEntity*>> mEntitiesByClass;

	std::unordered_map<std::type_index, mage::Array<GameWorldComponent*>> mEntityCallbacks;
};

template <WorldComponentType Prerequisite>
class GameSystemPrerequisite
{
public:
	GameSystemPrerequisite(GameWorld& inWorld) : mPrerequisite(*inWorld.GetComponent<Prerequisite>()) {}

	Prerequisite& Get() const { return mPrerequisite; }

protected:
	~GameSystemPrerequisite() {}

private:
	Prerequisite& mPrerequisite;
};

template <WorldComponentType Prerequisite, typename... OtherPrerequisites>
bool CheckGameSystemPrerequisites(GameWorld& inWorld)
{
	if constexpr (sizeof...(OtherPrerequisites) > 0)
	{
		return (inWorld.GetComponent<Prerequisite>() != nullptr)
			&& (CheckGameSystemPrerequisites<OtherPrerequisites...>(inWorld));
	}
	else
	{
		return (inWorld.GetComponent<Prerequisite>() != nullptr);
	}
}

template <typename... Prerequisites>
class GameSystemWithPrerequisites : public GameSystem, public GameSystemPrerequisite<Prerequisites>...
{
public:
	static bool CheckPrerequisites(GameWorld& inWorld)
	{
		return CheckGameSystemPrerequisites<Prerequisites...>(inWorld);
	}

	GameSystemWithPrerequisites(GameWorld& inWorld) : GameSystem(inWorld),
		GameSystemPrerequisite<Prerequisites>(inWorld)... {}

	template <typename Prerequisite>
	Prerequisite& Get() const { return GameSystemPrerequisite<Prerequisite>::Get(); }

	virtual ~GameSystemWithPrerequisites() {}
};

template <AssetType Type>
struct Property<AssetHandle<Type>> : public PropertyNode
{
	virtual void SetValue(void* inMemory, mage::StringView inValue) const override
	{
		*(AssetHandle<Type>*)(inMemory) = gEngine->mAssetManager.FindAsset<Type>(inValue);
	}

	virtual void GetValue(void* inMemory, mage::String& outValue) const override
	{
		outValue = ((AssetHandle<Type>*)(inMemory))->GetName();
	}

	static void GetChildProperties(PropertyTree& inOutContainer) {}
};
