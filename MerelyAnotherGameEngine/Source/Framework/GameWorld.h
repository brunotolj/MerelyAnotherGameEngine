#pragma once

#include <typeindex>

class GameObject;
class GameWorld;

class GameWorldComponent : public NonMovable
{
public:
	static bool CheckPrerequisites(GameWorld& inWorld) { return true; }

	GameWorldComponent(GameWorld& inWorld) : mWorld(inWorld) {}
	virtual ~GameWorldComponent() {}

protected:
	GameWorld& mWorld;
};

class GameUtility : public GameWorldComponent
{
public:
	GameUtility(GameWorld& inWorld) : GameWorldComponent(inWorld) {}

	virtual void PreSystemsUpdate() {};
	virtual void PostSystemsUpdate() {};
};

class GameSystem : public GameWorldComponent
{
public:
	GameSystem(GameWorld& inWorld) : GameWorldComponent(inWorld) {}

	virtual void Update(f32 inDeltaTime) {};
};

class GameWorld : public NonCopyable
{
public:
	GameWorld() {};
	~GameWorld();

	void Update(f32 inDeltaTime);

	void ForEachObject(std::function<bool(GameObject*)> inPredicate);

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

		return reinterpret_cast<ComponentClass*>(component->second);
	}

	template <typename ObjectClass, typename... Args>
	ObjectClass* CreateObject(Args&&... inArgs)
	{
		ObjectClass* object = new ObjectClass(*this, inArgs...);

		if (mIsCurrentlyUpdatingObjects)
			mNewObjects.Add(object);
		else
			mObjects.Add(object);

		object->OnAddedToWorld();
		return object;
	}

private:
	mage::Array<GameUtility*> mUtilities;
	mage::Array<GameSystem*> mSystems;
	std::unordered_map<std::type_index, GameWorldComponent*> mComponentByClass;

	mage::Array<GameObject*> mObjects;
	mage::Array<GameObject*> mNewObjects;

	bool mIsCurrentlyUpdatingObjects = false;
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
