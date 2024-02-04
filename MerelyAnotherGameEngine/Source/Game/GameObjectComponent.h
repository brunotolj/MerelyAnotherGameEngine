#pragma once

#include "Core/NonCopyable.h"

class GameObject;
class GameWorld;

class GameObjectComponentBase : public NonCopyableClass
{
	friend GameObject;

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) {}

	virtual void OnOwnerRemovedFromWorld(GameWorld& world) {}

	virtual void UpdatePrePhysics(float deltaTime) {}

	virtual void UpdatePostPhysics(float deltaTime) {}
};

template <typename T>
class GameObjectComponent : public GameObjectComponentBase
{
	friend GameObject;

public:
	GameObjectComponent(T& owner) : mOwner(owner) {}

protected:
	T& mOwner;
};
