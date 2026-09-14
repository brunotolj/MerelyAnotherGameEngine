#pragma once

#include "Framework/GameWorld.h"
#include "Framework/TransformTree.h"

class FreeMoveSystem : public GameSystemWithPrerequisites<TransformTree>
{
public:
	FreeMoveSystem(GameWorld& inWorld);

	virtual void Update(f32 inDeltaTime) override;

	void Setup(TransformTreeEntryId inTransformId, f32 inSpeed);

private:
	TransformTreeEntryId mTransformId;
	f32 mSpeed;

	glm::dvec2 mCursorMovement = glm::dvec2(0.0f);
	glm::vec2 mRotation = glm::vec2(0.0f);
};
