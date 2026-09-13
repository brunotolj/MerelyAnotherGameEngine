#include "Framework/Systems/WorldBoundsSystem.h"
#include "Game/GameObject.h"

WorldBoundsSystem::WorldBoundsSystem(GameWorld& inWorld, glm::vec3 inBoundsMin, glm::vec3 inBoundsMax)
	: GameSystemWithPrerequisites(inWorld), mBoundsMin(inBoundsMin), mBoundsMax(inBoundsMax)
{
	mage_check(inBoundsMin.x <= inBoundsMax.x);
	mage_check(inBoundsMin.y <= inBoundsMax.y);
	mage_check(inBoundsMin.z <= inBoundsMax.z);
}

void WorldBoundsSystem::Update(f32 inDeltaTime)
{
	mWorld.ForEachObject([&](GameObject* object)
	{
		if (TransformableObject* transformableObject = dynamic_cast<TransformableObject*>(object))
		{
			TransformTreeEntryId transformId = transformableObject->GetTransformId();
			glm::vec3 position = Get<TransformTree>().GetGlobalTransform(transformId).Position;

			bool outOfBounds = false;
			outOfBounds |= position.x < mBoundsMin.x;
			outOfBounds |= position.y < mBoundsMin.y;
			outOfBounds |= position.z < mBoundsMin.z;
			outOfBounds |= position.x > mBoundsMax.x;
			outOfBounds |= position.y > mBoundsMax.y;
			outOfBounds |= position.z > mBoundsMax.z;

			if (outOfBounds)
				object->Destroy();
		}

		return mage::Continue;
	});
}
