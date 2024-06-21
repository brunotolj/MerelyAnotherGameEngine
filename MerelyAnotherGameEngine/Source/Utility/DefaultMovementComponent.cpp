#include "Game/GameWorld.h"
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
	world.GetInputSystem().BindCursorMovementHandler([this](glm::dvec2 movement, int cursorMode)
		{ if (cursorMode == GLFW_CURSOR_DISABLED) mCursorMovement += movement; });
}

void DefaultMovementComponent::UpdatePrePhysics(float deltaTime)
{
	glm::vec3 movement(0.0f);
	InputSystem& inputSystem = mOwner.GetWorld()->GetInputSystem();

	mRotation += 0.01f * glm::vec2(mCursorMovement);
	mRotation.y = glm::clamp(mRotation.y, -glm::radians(80.0f), glm::radians(80.0f));
	mCursorMovement = glm::dvec2(0.0f);

	mOwner.Transform.Rotation = mage::Rotor::Combine(
		mage::Rotor::FromAxisAndAngle({ 0.0f, 0.0f, 1.0f }, mRotation.x),
		mage::Rotor::FromAxisAndAngle({ 1.0f, 0.0f, 0.0f }, mRotation.y));

	if (inputSystem.GetKeyState(GLFW_KEY_D) == GLFW_PRESS) movement.x += 1.0f;
	if (inputSystem.GetKeyState(GLFW_KEY_A) == GLFW_PRESS) movement.x -= 1.0f;

	if (inputSystem.GetKeyState(GLFW_KEY_W) == GLFW_PRESS) movement.y += 1.0f;
	if (inputSystem.GetKeyState(GLFW_KEY_S) == GLFW_PRESS) movement.y -= 1.0f;

	movement = mOwner.Transform.Matrix() * glm::vec4(movement, 0.0f);

	if (inputSystem.GetKeyState(GLFW_KEY_E) == GLFW_PRESS) movement.z += 1.0f;
	if (inputSystem.GetKeyState(GLFW_KEY_Q) == GLFW_PRESS) movement.z -= 1.0f;

	mOwner.Transform.Position += mSpeed * deltaTime * movement;
}
