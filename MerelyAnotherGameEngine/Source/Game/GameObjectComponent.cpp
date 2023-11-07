#include "Game/GameObjectComponent.h"

GameObjectComponent::GameObjectComponent(GameObject& owner) :
	mOwner(owner)
{
}

void GameObjectComponent::OnOwnerAddedToWorld(GameWorld& world)
{
}

void GameObjectComponent::OnOwnerRemovedFromWorld(GameWorld& world)
{
}

void GameObjectComponent::UpdatePrePhysics(float deltaTime)
{
}

void GameObjectComponent::UpdatePostPhysics(float deltaTime)
{
}
