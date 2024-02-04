#include "Game/GameObject.h"
#include "Core/Asserts.h"
#include "Game/GameObjectComponent.h"

void GameObject::OnAddedToWorld(GameWorld& world)
{
	mWorld = &world;

	for (const std::unique_ptr<GameObjectComponentBase>& component : mComponents)
	{
		component->OnOwnerAddedToWorld(world);
	}
}

void GameObject::OnRemovedFromWorld(GameWorld& world)
{
	mage_check(&world == mWorld);
	mWorld = nullptr;

	for (const std::unique_ptr<GameObjectComponentBase>& component : mComponents)
	{
		component->OnOwnerRemovedFromWorld(world);
	}
}

void GameObject::UpdatePrePhysics(float deltaTime)
{
	for (const std::unique_ptr<GameObjectComponentBase>& component : mComponents)
	{
		component->UpdatePrePhysics(deltaTime);
	}
}

void GameObject::UpdatePostPhysics(float deltaTime)
{
	for (const std::unique_ptr<GameObjectComponentBase>& component : mComponents)
	{
		component->UpdatePostPhysics(deltaTime);
	}
}
