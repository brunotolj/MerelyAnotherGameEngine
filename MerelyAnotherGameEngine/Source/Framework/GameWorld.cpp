#include "Engine/Engine.h"
#include "Framework/GameWorld.h"

std::unordered_map<mage::String, WorldComponentFactoryCallback> gWorldComponentFactoryFunctions;

GameWorld::GameWorld(WorldSetup const& inWorldSetup)
{
	for (WorldComponentSetup const& componentSetup : inWorldSetup.mComponentSetups)
	{
		if (gWorldComponentFactoryFunctions.contains(componentSetup.Type) == false)
			continue;

		gWorldComponentFactoryFunctions[componentSetup.Type](*this, componentSetup.Properties);
	}

	mage::Array<GameEntity*> parentChain;
	parentChain.Add(nullptr);

	for (EntitySetup const& entitySetup : inWorldSetup.mEntitySetups)
	{
		if (gEntityFactoryFunctions.contains(entitySetup.Type) == false)
			continue;

		if (mage_ensure(entitySetup.ParentChainDepth < parentChain.GetSize()) == false)
			continue;

		parentChain.ResizeUninitialized(entitySetup.ParentChainDepth + 1);

		GameEntity* entity = gEntityFactoryFunctions[entitySetup.Type](*this, parentChain[entitySetup.ParentChainDepth], entitySetup.Properties);
		parentChain.Add(entity);
	}
}

GameWorld::~GameWorld()
{
	gEngine->mVulkanDevice.GetVkDevice().waitIdle();

	while (mEntities.GetSize() > 0)
		DestroyEntity(mEntities.GetLast());

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

	u32 destroyedEntities = 0;
	for (u32 i = 0; i < mEntities.GetSize() - destroyedEntities; ++i)
	{
		if (!mEntities[i]->IsDestroyed())
			continue;

		destroyedEntities++;
		mEntities.Swap(i--, mEntities.GetSize() - destroyedEntities);
	}

	u32 remainingEntities = mEntities.GetSize() - destroyedEntities;
	while (mEntities.GetSize() > remainingEntities)
		DestroyEntity(mEntities.GetLast());
}

void GameWorld::DestroyEntity(GameEntity* inEntity)
{
	for (GameEntity* childEntity : inEntity->mChildEntities)
		DestroyEntity(childEntity);

	mEntities.RemoveSwap(inEntity);
	mEntitiesByClass.at(inEntity->mTypeIndex).RemoveSwap(inEntity);

	delete inEntity;
}
