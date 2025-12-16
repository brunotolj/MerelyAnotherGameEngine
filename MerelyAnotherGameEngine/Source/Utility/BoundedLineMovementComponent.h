#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

template<>
struct ComponentTemplate<class BoundedLineMovementComponent>
{
	glm::vec3 Extent;

	i32 InputNeg = GLFW_KEY_LEFT;
	i32 InputPos = GLFW_KEY_RIGHT;

	f32 Acceleration = 1.0f;
	f32 Deceleration = 1.0f;
	f32 MaxSpeed = 1.0f;
};

class BoundedLineMovementComponent : public GameObjectComponent<TransformableObject>
{
public:
	BoundedLineMovementComponent(TransformableObject& owner, const ComponentTemplate<BoundedLineMovementComponent>& creationTemplate);

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) override final;

	virtual void UpdatePrePhysics(f32 deltaTime) override final;

private:
	glm::vec3 mCenter;
	glm::vec3 mExtent;

	i32 mInputNeg;
	i32 mInputPos;

	f32 mAcceleration;
	f32 mDeceleration;
	f32 mMaxSpeed;

	f32 mSpeed = 0.0f;
	f32 mPosition = 0.0f;
	f32 mLength;
};
