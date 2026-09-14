#include "Framework/GameWorld.h"
#include "Game/GameObject.h"

GameWorld::~GameWorld()
{
	for (GameObject* object : mObjects)
	{
		object->Destroy();
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

	for (GameSystem* system : mSystems)
		system->Update(inDeltaTime);

	for (GameUtility* utility : mUtilities)
		utility->PostSystemsUpdate();

	for (u32 i = 0; i < mObjects.GetSize(); ++i)
	{
		if (!mObjects[i]->mIsDestoryed)
			continue;

		delete mObjects[i];
		mObjects.RemoveAtSwap(i--);
	}
}

void GameWorld::ForEachObject(std::function<mage::BreakOrContinue(GameObject*)> inPredicate)
{
	for (GameObject* object : mObjects)
	{
		if (object->IsDestroyed())
			continue;

		if (inPredicate(object) == mage::Break)
			break;
	}
}
