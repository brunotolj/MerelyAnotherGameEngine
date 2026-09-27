#pragma once

#include "Framework/GameWorld.h"
#include "Framework/TransformTree.h"

class WorldBoundsSystem : public GameSystemWithPrerequisites<TransformTree>
{
public:
	WorldBoundsSystem(GameWorld& inWorld);

	virtual void Update(f32 inDeltaTime) override;

	glm::vec3 BoundsMin{ -10000.0f, -10000.0f, -10000.0f };
	glm::vec3 BoundsMax{ 10000.0f, 10000.0f, 10000.0f };
};

template <>
struct Property<WorldBoundsSystem> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(WorldBoundsSystem, BoundsMin);
		inOutContainer.AddProperty(WorldBoundsSystem, BoundsMax);
	}
};
