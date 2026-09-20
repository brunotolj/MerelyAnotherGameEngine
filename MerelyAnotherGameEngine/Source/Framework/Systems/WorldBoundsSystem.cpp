#include "Framework/Systems/WorldBoundsSystem.h"

WorldBoundsSystem::WorldBoundsSystem(GameWorld& inWorld, glm::vec3 inBoundsMin, glm::vec3 inBoundsMax)
	: GameSystemWithPrerequisites(inWorld), mBoundsMin(inBoundsMin), mBoundsMax(inBoundsMax)
{
	mage_check(inBoundsMin.x <= inBoundsMax.x);
	mage_check(inBoundsMin.y <= inBoundsMax.y);
	mage_check(inBoundsMin.z <= inBoundsMax.z);
}

void WorldBoundsSystem::Update(f32 inDeltaTime)
{
	for (TransformEntity* transformEntity : mWorld.GetEntities<TransformEntity>())
	{
		TransformTreeEntryId transformId = transformEntity->mTransformId;
		glm::vec3 position = Get<TransformTree>().GetGlobalTransform(transformId).Position;

		bool outOfBounds = false;
		outOfBounds |= position.x < mBoundsMin.x;
		outOfBounds |= position.y < mBoundsMin.y;
		outOfBounds |= position.z < mBoundsMin.z;
		outOfBounds |= position.x > mBoundsMax.x;
		outOfBounds |= position.y > mBoundsMax.y;
		outOfBounds |= position.z > mBoundsMax.z;

		if (outOfBounds)
			transformEntity->MarkDestroyed();
	}
}
