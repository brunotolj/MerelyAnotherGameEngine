#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"

GameObject::~GameObject()
{
	for (GameObjectComponentBase* component : mComponents)
	{
		delete component;
	}
}

void GameObject::OnAddedToWorld(GameWorld& world)
{
	mWorld = &world;

	for (GameObjectComponentBase* component : mComponents)
	{
		component->OnOwnerAddedToWorld(world);
	}
}

void GameObject::OnRemovedFromWorld(GameWorld& world)
{
	mage_check(&world == mWorld);
	mWorld = nullptr;

	for (GameObjectComponentBase* component : mComponents)
	{
		component->OnOwnerRemovedFromWorld(world);
	}
}

void GameObject::UpdatePrePhysics(f32 deltaTime)
{
	for (GameObjectComponentBase* component : mComponents)
	{
		component->UpdatePrePhysics(deltaTime);
	}
}

void GameObject::UpdatePostPhysics(f32 deltaTime)
{
	for (GameObjectComponentBase* component : mComponents)
	{
		component->UpdatePostPhysics(deltaTime);
	}
}
