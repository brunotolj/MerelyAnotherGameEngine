#pragma once

#include "Game/GameObjectCommon.h"

class GameObject;
class GameWorld;

class GameObjectComponentBase : public NonCopyableClass
{
	friend GameObject;

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) {}

	virtual void OnOwnerRemovedFromWorld(GameWorld& world) {}

	virtual void UpdatePrePhysics(f32 deltaTime) {}

	virtual void UpdatePostPhysics(f32 deltaTime) {}
};

template<GameObjectClass OwnerClass>
class GameObjectComponent : public GameObjectComponentBase
{
	friend GameObject;

public:
	GameObjectComponent(OwnerClass& owner) : mOwner(owner) {}

protected:
	OwnerClass& mOwner;
};
