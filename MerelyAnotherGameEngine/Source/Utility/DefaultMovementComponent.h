#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

template<>
struct ComponentTemplate<class DefaultMovementComponent>
{
	i32 InputR = GLFW_KEY_D;
	i32 InputL = GLFW_KEY_A;
	i32 InputF = GLFW_KEY_W;
	i32 InputB = GLFW_KEY_S;
	i32 InputU = GLFW_KEY_E;
	i32 InputD = GLFW_KEY_Q;

	f32 Speed = 1.0f;
};

class DefaultMovementComponent : public GameObjectComponent<TransformableObject>
{
public:
	DefaultMovementComponent(TransformableObject& owner, const ComponentTemplate<DefaultMovementComponent>& creationTemplate);

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) override final;

	virtual void UpdatePrePhysics(f32 deltaTime) override final;

private:
	i32 mInputR;
	i32 mInputL;
	i32 mInputF;
	i32 mInputB;
	i32 mInputU;
	i32 mInputD;

	f32 mSpeed;

	glm::dvec2 mCursorMovement = glm::dvec2(0.0f);
	glm::vec2 mRotation = glm::vec2(0.0f);
};
