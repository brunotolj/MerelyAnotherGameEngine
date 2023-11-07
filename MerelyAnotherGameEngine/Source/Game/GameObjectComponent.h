#pragma once

#include "Core/NonCopyable.h"

class GameObject;
class GameWorld;

class GameObjectComponent : public NonCopyableClass
{
	friend GameObject;

public:
	GameObjectComponent(GameObject& owner);

protected:
	GameObject& mOwner;

	virtual void OnOwnerAddedToWorld(GameWorld& world);

	virtual void OnOwnerRemovedFromWorld(GameWorld& world);

	virtual void UpdatePrePhysics(float deltaTime);

	virtual void UpdatePostPhysics(float deltaTime);
};
