#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

template<>
struct ComponentTemplate<class DefaultMovementComponent>
{
	int InputR = GLFW_KEY_D;
	int InputL = GLFW_KEY_A;
	int InputF = GLFW_KEY_W;
	int InputB = GLFW_KEY_S;
	int InputU = GLFW_KEY_E;
	int InputD = GLFW_KEY_Q;

	float Speed = 1.0f;
};

class DefaultMovementComponent : public GameObjectComponent<TransformableObject>
{
public:
	DefaultMovementComponent(TransformableObject& owner, const ComponentTemplate<DefaultMovementComponent>& creationTemplate);

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) override final;

	virtual void UpdatePrePhysics(float deltaTime) override final;

private:
	int mInputR;
	int mInputL;
	int mInputF;
	int mInputB;
	int mInputU;
	int mInputD;

	float mSpeed;

	glm::dvec2 mCursorMovement = glm::dvec2(0.0f);
	glm::vec2 mRotation = glm::vec2(0.0f);
};
