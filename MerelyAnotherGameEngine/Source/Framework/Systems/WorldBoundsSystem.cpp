#include "Framework/Systems/WorldBoundsSystem.h"

REGISTER_WORLD_COMPONENT_FACTORY_FUNCTION(WorldBoundsSystem);

WorldBoundsSystem::WorldBoundsSystem(GameWorld& inWorld) : GameSystemWithPrerequisites(inWorld)
{
}

void WorldBoundsSystem::Update(f32 inDeltaTime)
{
	for (TransformEntity* transformEntity : mWorld.GetEntities<TransformEntity>())
	{
		TransformTreeEntryId transformId = transformEntity->mTransformId;
		glm::vec3 position = Get<TransformTree>().GetGlobalTransform(transformId).Position;

		bool outOfBounds = false;
		outOfBounds |= position.x < BoundsMin.x;
		outOfBounds |= position.y < BoundsMin.y;
		outOfBounds |= position.z < BoundsMin.z;
		outOfBounds |= position.x > BoundsMax.x;
		outOfBounds |= position.y > BoundsMax.y;
		outOfBounds |= position.z > BoundsMax.z;

		if (outOfBounds)
			transformEntity->MarkDestroyed();
	}
}
