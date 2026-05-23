#include "Game/GameWorld.h"
#include "Engine/WindowManager.h"
#include "Game/InputSystem.h"
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
	world.mInputSystem.BindCursorMovementHandler([this](glm::dvec2 movement, CursorInputMode cursorMode)
		{ if (cursorMode == CursorInputMode::Disabled) mCursorMovement += movement; });
}

void DefaultMovementComponent::UpdatePrePhysics(f32 deltaTime)
{
	glm::vec3 movement(0.0f);
	InputSystem& inputSystem = mOwner.GetWorld()->mInputSystem;

	mRotation += 0.01f * glm::vec2(mCursorMovement);
	mRotation.y = glm::clamp(mRotation.y, -glm::radians(80.0f), glm::radians(80.0f));
	mCursorMovement = glm::dvec2(0.0f);

	mOwner.Transform.Rotation = mage::Rotor::Combine(
		mage::Rotor({ 0.0f, 0.0f, 1.0f }, mRotation.x),
		mage::Rotor({ 1.0f, 0.0f, 0.0f }, mRotation.y));

	if (inputSystem.IsKeyPressed(GLFW_KEY_D)) movement.x += 1.0f;
	if (inputSystem.IsKeyPressed(GLFW_KEY_A)) movement.x -= 1.0f;

	if (inputSystem.IsKeyPressed(GLFW_KEY_W)) movement.y += 1.0f;
	if (inputSystem.IsKeyPressed(GLFW_KEY_S)) movement.y -= 1.0f;

	movement = mOwner.Transform.Matrix() * glm::vec4(movement, 0.0f);

	if (inputSystem.IsKeyPressed(GLFW_KEY_E)) movement.z += 1.0f;
	if (inputSystem.IsKeyPressed(GLFW_KEY_Q)) movement.z -= 1.0f;

	mOwner.Transform.Position += mSpeed * deltaTime * movement;
}
