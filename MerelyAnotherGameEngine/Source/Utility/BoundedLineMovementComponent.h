#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

template<>
struct ComponentTemplate<class BoundedLineMovementComponent>
{
	glm::vec3 Extent;

	int InputNeg = GLFW_KEY_LEFT;
	int InputPos = GLFW_KEY_RIGHT;

	float Acceleration = 1.0f;
	float Deceleration = 1.0f;
	float MaxSpeed = 1.0f;
};

class BoundedLineMovementComponent : public GameObjectComponent<TransformableObject>
{
public:
	BoundedLineMovementComponent(TransformableObject& owner, const ComponentTemplate<BoundedLineMovementComponent>& creationTemplate);

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) override final;

	virtual void UpdatePrePhysics(float deltaTime) override final;

private:
	glm::vec3 mCenter;
	glm::vec3 mExtent;

	int mInputNeg;
	int mInputPos;

	float mAcceleration;
	float mDeceleration;
	float mMaxSpeed;

	float mSpeed = 0.0f;
	float mPosition = 0.0f;
	float mLength;
};
