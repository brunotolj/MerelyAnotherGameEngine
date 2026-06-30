#pragma once

#include "Game/GameObjectCommon.h"

#include <map>
#include <typeindex>

class GameObjectComponentBase;
class GameWorld;

class GameObject : public NonCopyableClass
{
	friend GameWorld;

public:
	template<GameObjectComponentClass ComponentClass>
	mage::Array<ComponentClass*> GetComponentsOfClass()
	{
		mage::Array<ComponentClass*> result;

		auto componentArray = mComponentsByClass.find(typeid(ComponentClass));

		if (componentArray == mComponentsByClass.end())
			return result;

		for (u32 index : componentArray->second)
			result.Add((ComponentClass*)mComponents[index]);

		return result;
	}

	template<GameObjectComponentClass ComponentClass>
	mage::Array<ComponentClass const*> GetComponentsOfClass() const
	{
		mage::Array<ComponentClass const*> result;

		auto componentArray = mComponentsByClass.find(typeid(ComponentClass));

		if (componentArray == mComponentsByClass.end())
			return result;

		for (u32 index : componentArray->second)
			result.Add((ComponentClass*)mComponents[index]);

		return result;
	}

	template<typename... ComponentTemplates>
	GameObject(ComponentTemplates... inComponents)
	{
		(CreateComponent(*this, inComponents), ...);
	}

	~GameObject();

	bool IsDestroyed() const { return mIsDestoryed; }
	void Destroy() { mIsDestoryed = true; }

	GameWorld* GetWorld() const { return mWorld; }

protected:
	template<GameObjectClass ObjectClass, GameObjectComponentClass ComponentClass>
	static ComponentClass& CreateComponent(ObjectClass& owner, const ComponentTemplate<ComponentClass>& creationTemplate)
	{
		ComponentClass* component = new ComponentClass(owner, creationTemplate);

		u32 index = owner.mComponents.GetSize();
		owner.mComponents.Add(component);
		owner.mComponentsByClass[typeid(ComponentClass)].Add(index);

		return *component;
	}

	void OnAddedToWorld(GameWorld& world);

	void OnRemovedFromWorld(GameWorld& world);
	
	void UpdatePrePhysics(f32 deltaTime);

	void UpdatePostPhysics(f32 deltaTime);

private:
	GameWorld* mWorld = nullptr;

	mage::Array<GameObjectComponentBase*> mComponents;

	std::map<std::type_index, mage::Array<u32>> mComponentsByClass;

	bool mIsDestoryed = false;
};

class TransformableObject : public GameObject
{
public:
	template<typename... ComponentTemplates>
	TransformableObject(mage::Transform inInitialTransform, ComponentTemplates... inComponents)
	{
		mTransform = inInitialTransform;
		(CreateComponent(*this, inComponents), ...);
	}

	mage::Transform mTransform;
};
