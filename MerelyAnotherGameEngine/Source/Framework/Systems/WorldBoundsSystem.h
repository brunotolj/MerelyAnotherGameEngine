#pragma once

#include "Framework/GameWorld.h"
#include "Framework/TransformTree.h"

class WorldBoundsSystem : public GameSystemWithPrerequisites<TransformTree>
{
public:
	WorldBoundsSystem(GameWorld& inWorld, glm::vec3 inBoundsMin, glm::vec3 inBoundsMax);

	virtual void Update(f32 inDeltaTime) override;

private:
	glm::vec3 mBoundsMin;
	glm::vec3 mBoundsMax;
};
