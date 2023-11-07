#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/RenderSystem.h"

void GameWorld::Update(float deltaTime)
{
	for (const std::shared_ptr<GameObject>& object : mObjects)
	{
		object->UpdatePrePhysics(deltaTime);
	}

	mPhysicsSystem->Update(deltaTime);

	for (const std::shared_ptr<GameObject>& object : mObjects)
	{
		object->UpdatePostPhysics(deltaTime);
	}
}

void GameWorld::AddObject(const std::shared_ptr<GameObject>& object)
{
	mObjects.push_back(object);
	object->OnAddedToWorld(*this);
}

void GameWorld::RemoveObject(const std::shared_ptr<GameObject>& object)
{
	object->OnRemovedFromWorld(*this);
}
