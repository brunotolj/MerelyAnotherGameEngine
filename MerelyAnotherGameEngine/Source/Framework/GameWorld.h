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

	template <WorldComponentType ComponentClass>
	ComponentClass* CreateComponent(PropertyValueMap const& inProperties)
	{
		if (GetComponent<ComponentClass>()) return nullptr;
		if (ComponentClass::CheckPrerequisites(*this) == false) return nullptr;

		ComponentClass* component = new ComponentClass(*this);
		PropertyTree(*component).ApplyPropertyValues(inProperties);

		mComponentByClass[typeid(ComponentClass)] = component;

		if constexpr (std::is_base_of<GameUtility, ComponentClass>::value)
			mUtilities.Add(component);
		else if constexpr (std::is_base_of<GameSystem, ComponentClass>::value)
			mSystems.Add(component);
		else
			static_assert(false, "Component is neither a GameUtility or GameSystem");
		
		return component;
	}

	template <WorldComponentType ComponentClass>
	ComponentClass* GetComponent()
	{
		auto component = mComponentByClass.find(typeid(ComponentClass));
		if (component == mComponentByClass.end())
			return nullptr;

		return (ComponentClass*)(component->second);
	}

	template <EntityType EntityClass, typename... Args>
	EntityClass* CreateEntity(GameEntity* inParentEntity, Args&&... inArgs)
	{
		if (EntityClass::IsParentEntityValid(inParentEntity) == false)
			return nullptr;

		EntityClass* entity = new EntityClass(*this, typeid(EntityClass), inParentEntity, std::forward<Args>(inArgs)...);
		mEntities.Add(entity);
		mEntitiesByClass[entity->mTypeIndex].Add(entity);
		return entity;
	}

	mage::Array<GameEntity*> const& GetEntities() const { return mEntities; }

	template <EntityType EntityClass>
	mage::Array<EntityClass*> const& GetEntities() const
	{
		static mage::Array<EntityClass*> dummy;
		if (mEntitiesByClass.contains(typeid(EntityClass)) == false) return dummy;
		return (mage::Array<EntityClass*> const&)(mEntitiesByClass.at(typeid(EntityClass)));
	}

private:
	void DestroyEntity(GameEntity* inEntity);

	mage::Array<GameUtility*> mUtilities;
	mage::Array<GameSystem*> mSystems;
	std::unordered_map<std::type_index, GameWorldComponent*> mComponentByClass;

	mage::Array<GameEntity*> mEntities;
	std::unordered_map<std::type_index, mage::Array<GameEntity*>> mEntitiesByClass;
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
