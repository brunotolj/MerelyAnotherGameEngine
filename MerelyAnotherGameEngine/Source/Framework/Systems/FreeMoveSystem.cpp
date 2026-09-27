#include "Framework/Systems/FreeMoveSystem.h"
#include "Engine/Engine.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

REGISTER_WORLD_COMPONENT_FACTORY_FUNCTION(FreeMoveSystem);

FreeMoveSystem::FreeMoveSystem(GameWorld& inWorld) : GameSystemWithPrerequisites(inWorld)
{
	gEngine->mInputHandler.BindCursorMovementHandler([&](glm::dvec2 movement, CursorInputMode cursorMode)
	{
		if (cursorMode == CursorInputMode::Disabled)
			mCursorMovement += movement;
	});
}

void FreeMoveSystem::Update(f32 inDeltaTime)
{
	glm::vec3 movement(0.0f);

	mRotation += 0.01f * glm::vec2(mCursorMovement);
	mRotation.y = glm::clamp(mRotation.y, -glm::radians(80.0f), glm::radians(80.0f));
	mCursorMovement = glm::dvec2(0.0f);

	TransformTreeEntryId targetTransformId = mage::InvalidIndex;
	for (FreeMoveTargetEntity* targetEntity : mWorld.GetEntities<FreeMoveTargetEntity>())
	{
		targetTransformId = targetEntity->GetParentEntity().mTransformId;
		break;
	}

	if (targetTransformId == mage::InvalidIndex)
		return;

	mage::Transform transform = Get<TransformTree>().GetGlobalTransform(targetTransformId);

	transform.Rotation = mage::Rotor::Combine(
		mage::Rotor({ 0.0f, 0.0f, 1.0f }, mRotation.x),
		mage::Rotor({ 1.0f, 0.0f, 0.0f }, mRotation.y));

	if (gEngine->mInputHandler.IsKeyPressed(GLFW_KEY_D)) movement.x += 1.0f;
	if (gEngine->mInputHandler.IsKeyPressed(GLFW_KEY_A)) movement.x -= 1.0f;

	if (gEngine->mInputHandler.IsKeyPressed(GLFW_KEY_W)) movement.y += 1.0f;
	if (gEngine->mInputHandler.IsKeyPressed(GLFW_KEY_S)) movement.y -= 1.0f;

	movement = transform.Matrix() * glm::vec4(movement, 0.0f);

	if (gEngine->mInputHandler.IsKeyPressed(GLFW_KEY_E)) movement.z += 1.0f;
	if (gEngine->mInputHandler.IsKeyPressed(GLFW_KEY_Q)) movement.z -= 1.0f;

	transform.Position += Speed * inDeltaTime * movement;

	Get<TransformTree>().SetGlobalTransform(targetTransformId, transform);
}
