#pragma once

#include "Assets/WorldSetup.h"
#include "Framework/Entity.h"
#include "Framework/GameWorldComponent.h"

#include <typeindex>

class GameWorld : public NonCopyable
{
public:
	GameWorld(WorldSetup const& inWorldSetup);
	~GameWorld();

	void Update(f32 inDeltaTime);

	mage::Array<GameEntity*> const& GetEntities() const { return mEntities; }

	template <typename EntityClass>
	mage::Array<EntityClass*> const& GetEntities() const
	{
		return (mage::Array<EntityClass*> const&)(mEntitiesByClass.at(typeid(EntityClass)));
	}

	template <typename ComponentClass, typename... Args>
	ComponentClass* CreateComponent(Args&&... inArgs)
	{
		if (GetComponent<ComponentClass>()) return nullptr;
		if (ComponentClass::CheckPrerequisites(*this) == false) return nullptr;

		ComponentClass* component = new ComponentClass(*this, inArgs...);
		mComponentByClass[typeid(ComponentClass)] = component;

		if constexpr (std::is_base_of<GameUtility, ComponentClass>::value)
			mUtilities.Add(component);
		else if constexpr (std::is_base_of<GameSystem, ComponentClass>::value)
			mSystems.Add(component);
		else
			static_assert(false, "Component is neither a GameUtility or GameSystem");
		
		return component;
	}

	template <typename ComponentClass>
	ComponentClass* GetComponent()
	{
		auto component = mComponentByClass.find(typeid(ComponentClass));
		if (component == mComponentByClass.end())
			return nullptr;

		return (ComponentClass*)(component->second);
	}

	template <typename EntityClass, typename... Args>
	EntityClass* CreateEntity(Args&&... inArgs)
	{
		EntityClass* entity = new EntityClass(*this, typeid(EntityClass), inArgs...);
		mEntities.Add(entity);
		mEntitiesByClass[entity->mTypeIndex].Add(entity);
		return entity;
	}

private:
	void DestroyEntity(GameEntity* inEntity);

	mage::Array<GameUtility*> mUtilities;
	mage::Array<GameSystem*> mSystems;
	std::unordered_map<std::type_index, GameWorldComponent*> mComponentByClass;

	mage::Array<GameEntity*> mEntities;
	std::unordered_map<std::type_index, mage::Array<GameEntity*>> mEntitiesByClass;
};

template <typename Prerequisite>
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

template <typename Prerequisite, typename... OtherPrerequisites>
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
