#pragma once

#include "Game/GameObjectCommon.h"

class GameObject;
class GameWorld;

class GameObjectComponentBase : public NonCopyable
{
	friend GameObject;

protected:
	virtual void OnOwnerAddedToWorld() {}
	virtual void OnOwnerRemovedFromWorld() {}

	virtual void UpdatePrePhysics(f32 inDeltaTime) {}
	virtual void UpdatePostPhysics(f32 inDeltaTime) {}
};

template<GameObjectClass OwnerClass>
class GameObjectComponent : public GameObjectComponentBase
{
	friend GameObject;

public:
	GameObjectComponent(OwnerClass& inOwner) : mOwner(inOwner) {}

protected:
	OwnerClass& mOwner;
};
