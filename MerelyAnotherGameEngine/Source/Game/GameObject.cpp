#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"

void GameObject::OnAddedToWorld(GameWorld& world)
{
	mWorld = &world;

	for (const std::shared_ptr<GameObjectComponentBase>& component : mComponents)
	{
		component->OnOwnerAddedToWorld(world);
	}
}

void GameObject::OnRemovedFromWorld(GameWorld& world)
{
	mage_check(&world == mWorld);
	mWorld = nullptr;

	for (const std::shared_ptr<GameObjectComponentBase>& component : mComponents)
	{
		component->OnOwnerRemovedFromWorld(world);
	}
}

void GameObject::UpdatePrePhysics(f32 deltaTime)
{
	for (const std::shared_ptr<GameObjectComponentBase>& component : mComponents)
	{
		component->UpdatePrePhysics(deltaTime);
	}
}

void GameObject::UpdatePostPhysics(f32 deltaTime)
{
	for (const std::shared_ptr<GameObjectComponentBase>& component : mComponents)
	{
		component->UpdatePostPhysics(deltaTime);
	}
}
