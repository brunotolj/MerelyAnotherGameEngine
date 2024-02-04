#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/RenderSystem.h"

GameWorld::GameWorld(
	std::unique_ptr<RenderSystem>&& renderSystem,
	std::unique_ptr<PhysicsSystem>&& physicsSystem) :
	mRenderSystem(std::move(renderSystem)),
	mPhysicsSystem(std::move(physicsSystem))
{
}

void GameWorld::Update(float deltaTime)
{
	for (const std::shared_ptr<GameObject>& object : mObjects)
	{
		if (object->mIsDestoryed)
			continue;

		object->UpdatePrePhysics(deltaTime);
	}

	mPhysicsSystem->Update(deltaTime);

	for (const std::shared_ptr<GameObject>& object : mObjects)
	{
		if (object->mIsDestoryed)
			continue;

		object->UpdatePostPhysics(deltaTime);
	}

	size_t currentObject = 0;
	size_t destroyedObjectCount = 0;
	const size_t totalObjectCount = mObjects.size();

	while (currentObject + destroyedObjectCount < mObjects.size())
	{
		if (mObjects[currentObject]->mIsDestoryed)
		{
			std::swap(mObjects[currentObject], mObjects[totalObjectCount - destroyedObjectCount - 1]);
			destroyedObjectCount++;

			RemoveObject(mObjects[totalObjectCount - destroyedObjectCount]);
		}
		else
		{
			currentObject++;
		}
	}

	mObjects.resize(totalObjectCount - destroyedObjectCount);
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
