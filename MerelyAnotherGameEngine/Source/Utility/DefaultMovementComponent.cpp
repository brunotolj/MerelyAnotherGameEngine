#include "Game/GameWorld.h"
#include "Engine/Engine.h"
#include "Utility/DefaultMovementComponent.h"

DefaultMovementComponent::DefaultMovementComponent(TransformableObject& owner, const ComponentTemplate<DefaultMovementComponent>& creationTemplate) :
	GameObjectComponent(owner),
	mInputR(creationTemplate.InputR),
	mInputL(creationTemplate.InputL),
	mInputF(creationTemplate.InputF),
	mInputB(creationTemplate.InputB),
	mInputU(creationTemplate.InputU),
	mInputD(creationTemplate.InputD),
	mSpeed(creationTemplate.Speed)
{
}

void DefaultMovementComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	gEngine->mInputHandler.BindCursorMovementHandler([this](glm::dvec2 movement, CursorInputMode cursorMode)
		{ if (cursorMode == CursorInputMode::Disabled) mCursorMovement += movement; });
}

void DefaultMovementComponent::UpdatePrePhysics(f32 deltaTime)
{
	glm::vec3 movement(0.0f);

	mRotation += 0.01f * glm::vec2(mCursorMovement);
	mRotation.y = glm::clamp(mRotation.y, -glm::radians(80.0f), glm::radians(80.0f));
	mCursorMovement = glm::dvec2(0.0f);

	mage::Transform transform = mOwner.GetTransform();

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

	transform.Position += mSpeed * deltaTime * movement;

	mOwner.SetTransform(transform);
}
