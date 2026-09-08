#pragma once

#include "Framework/TransformTree.h"
#include "Game/GameObjectCommon.h"

#include <map>
#include <typeindex>

class GameObjectComponentBase;
class GameWorld;

class GameObject : public NonCopyable
{
	friend GameWorld;

public:
	template<typename... ComponentTemplates>
	GameObject(GameWorld& inWorld, ComponentTemplates... inComponents) : mWorld(inWorld)
	{
		(CreateComponent(*this, inComponents), ...);
	}

	~GameObject();
	
	template<GameObjectClass ObjectClass, GameObjectComponentClass ComponentClass>
	static ComponentClass& CreateComponent(ObjectClass& owner, const ComponentTemplate<ComponentClass>& creationTemplate)
	{
		ComponentClass* component = new ComponentClass(owner, creationTemplate);

		u32 index = owner.mComponents.GetSize();
		owner.mComponents.Add(component);
		owner.mComponentsByClass[typeid(ComponentClass)].Add(index);

		return *component;
	}

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

	bool IsDestroyed() const { return mIsDestoryed; }
	virtual void Destroy() { mIsDestoryed = true; }

	GameWorld& mWorld;

protected:
	void OnAddedToWorld();
	void OnRemovedFromWorld();
	
	void Update(f32 inDeltaTime);

private:
	mage::Array<GameObjectComponentBase*> mComponents;
	std::map<std::type_index, mage::Array<u32>> mComponentsByClass;

	bool mIsDestoryed = false;
};

class TransformableObject : public GameObject
{
public:
	template<typename... ComponentTemplates>
	TransformableObject(GameWorld& inWorld, mage::Transform inInitialTransform, ComponentTemplates... inComponents) : GameObject(inWorld)
	{
		(CreateComponent(*this, inComponents), ...);
		InitTransform(inInitialTransform);
	}

	TransformTreeEntryId GetTransformId() const { return mTransformId; }
	mage::Transform GetTransform() const;
	void SetTransform(mage::Transform inTransform);

	virtual void Destroy() override;

private:
	void InitTransform(mage::Transform inInitialTransform);

	TransformTreeEntryId mTransformId = mage::InvalidIndex;
};
