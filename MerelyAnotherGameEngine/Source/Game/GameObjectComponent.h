#pragma once

#include "Core/NonCopyable.h"
#include "Game/GameObjectCommon.h"

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

template<GameObjectClass OwnerClass>
class GameObjectComponent : public GameObjectComponentBase
{
	friend GameObject;

public:
	GameObjectComponent(OwnerClass& owner) : mOwner(owner) {}

protected:
	OwnerClass& mOwner;
};
