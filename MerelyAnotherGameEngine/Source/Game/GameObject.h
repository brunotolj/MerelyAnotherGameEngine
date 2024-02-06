#pragma once

#include "Core/NonCopyable.h"
#include "Core/Transform.h"
#include "Game/GameObjectCommon.h"

#include <memory>
#include <vector>

class GameObjectComponentBase;
class GameWorld;

class GameObject : public NonCopyableClass
{
	friend GameWorld;

public:
	template<GameObjectClass ObjectClass, GameObjectComponentClass ComponentClass>
	static ComponentClass& CreateComponent(ObjectClass& owner, const ComponentTemplate<ComponentClass>& creationTemplate)
	{
		std::unique_ptr<ComponentClass> component = std::make_unique<ComponentClass>(owner, creationTemplate);
		ComponentClass& componentRef = *component.get();
		owner.mComponents.push_back(std::move(component));

		return componentRef;
	}

	bool IsDestroyed() const { return mIsDestoryed; }

	void MarkDestroyed() { mIsDestoryed = true; }

protected:
	void OnAddedToWorld(GameWorld& world);

	void OnRemovedFromWorld(GameWorld& world);
	
	void UpdatePrePhysics(float deltaTime);

	void UpdatePostPhysics(float deltaTime);

private:
	GameWorld* mWorld = nullptr;

	std::vector<std::unique_ptr<GameObjectComponentBase>> mComponents;

	bool mIsDestoryed = false;
};

class TransformableObject : public GameObject
{
public:
	mage::Transform Transform;
};
