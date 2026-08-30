#include "Framework/GameWorld.h"
#include "Game/GameObject.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Systems/MeshRenderSystem.h"
#include "Rendering/Systems/SpriteRenderSystem.h"
#include "Rendering/Systems/TextRenderSystem.h"

GameWorld::~GameWorld()
{
	for (GameObject* object : mObjects)
	{
		object->Destroy();
		object->OnRemovedFromWorld();
		delete object;
	}

	mObjects.Empty();

	for (i32 i = mSystems.GetSize() - 1; i >= 0; --i)
		delete mSystems[i];

	for (i32 i = mUtilities.GetSize() - 1; i >= 0; --i)
		delete mUtilities[i];

	mSystems.Empty();
	mUtilities.Empty();
	mComponentByClass.clear();
}

void GameWorld::Update(f32 inDeltaTime)
{
	for (GameUtility* utility : mUtilities)
		utility->PreSystemsUpdate();

	mIsCurrentlyUpdatingObjects = true;
	for (GameObject* object : mObjects)
	{
		if (object->mIsDestoryed)
			continue;

		object->UpdatePrePhysics(inDeltaTime);
	}

	mIsCurrentlyUpdatingObjects = false;
	for (GameObject* newObject : mNewObjects)
	{
		newObject->UpdatePrePhysics(inDeltaTime);
		mObjects.Add(newObject);
	}
	mNewObjects.Empty();

	GetComponent<PhysicsSystem>()->Update(inDeltaTime);

	mIsCurrentlyUpdatingObjects = true;
	for (GameObject* object : mObjects)
	{
		if (object->mIsDestoryed)
			continue;

		object->UpdatePostPhysics(inDeltaTime);
	}

	mIsCurrentlyUpdatingObjects = false;
	for (GameObject* newObject : mNewObjects)
	{
		newObject->UpdatePostPhysics(inDeltaTime);
		mObjects.Add(newObject);
	}
	mNewObjects.Empty();

	for (u32 i = 0; i < mObjects.GetSize(); ++i)
	{
		if (!mObjects[i]->mIsDestoryed)
			continue;

		mObjects[i]->OnRemovedFromWorld();
		delete mObjects[i];
		mObjects.RemoveAtSwap(i--);
	}

	GetComponent<MeshRenderSystem>()->Update(inDeltaTime);
	GetComponent<SpriteRenderSystem>()->Update(inDeltaTime);
	GetComponent<TextRenderSystem>()->Update(inDeltaTime);

	for (GameUtility* utility : mUtilities)
		utility->PostSystemsUpdate();
}

void GameWorld::ForEachObject(std::function<bool(GameObject*)> inPredicate)
{
	for (GameObject* object : mObjects)
		if (!inPredicate(object))
			break;
}
