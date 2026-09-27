#pragma once

#include "Framework/GameWorld.h"
#include "Framework/TransformTree.h"

class FreeMoveSystem : public GameSystemWithPrerequisites<TransformTree>
{
public:
	FreeMoveSystem(GameWorld& inWorld);

	virtual void Update(f32 inDeltaTime) override;

	f32 Speed = 10.0f;

private:

	glm::dvec2 mCursorMovement = glm::dvec2(0.0f);
	glm::vec2 mRotation = glm::vec2(0.0f);
};

template <>
struct Property<FreeMoveSystem> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(FreeMoveSystem, Speed);
	}
};
