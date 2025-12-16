#include "Game/GameWorld.h"
#include "Game/InputSystem.h"
#include "Utility/BoundedLineMovementComponent.h"

BoundedLineMovementComponent::BoundedLineMovementComponent(TransformableObject& owner, const ComponentTemplate<BoundedLineMovementComponent>& creationTemplate) :
	GameObjectComponent(owner),
	mExtent(creationTemplate.Extent),
	mInputNeg(creationTemplate.InputNeg),
	mInputPos(creationTemplate.InputPos),
	mAcceleration(creationTemplate.Acceleration),
	mDeceleration(creationTemplate.Deceleration),
	mMaxSpeed(creationTemplate.MaxSpeed),
	mLength(glm::length(creationTemplate.Extent))
{
}

void BoundedLineMovementComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	mCenter = mOwner.Transform.Position;
}

void BoundedLineMovementComponent::UpdatePrePhysics(f32 deltaTime)
{
	InputSystem& inputSystem = mOwner.GetWorld()->GetInputSystem();

	f32 input = 0.0f;
	if (inputSystem.GetKeyState(mInputNeg) == GLFW_PRESS) input -= 1.0f;
	if (inputSystem.GetKeyState(mInputPos) == GLFW_PRESS) input += 1.0f;

	f32 remainingTime = deltaTime;
	f32 movement = 0.0f;

	if (remainingTime > 0.0f && mSpeed != 0.0f && input * mSpeed <= 0.0f)
	{
		const f32 decelTime = std::fabsf(mSpeed) / mDeceleration;
		if (decelTime > remainingTime)
		{
			const f32 deltaSpeed = mSpeed / std::fabsf(mSpeed) * mDeceleration * remainingTime;
			movement += (mSpeed - 0.5f * deltaSpeed) * remainingTime;
			mSpeed -= deltaSpeed;
			remainingTime = 0.0f;
		}
		else
		{
			movement += 0.5f * mSpeed * decelTime;
			mSpeed = 0.0f;
			remainingTime -= decelTime;
		}
	}

	if (remainingTime > 0.0f && ((mSpeed == 0.0f && input != 0.0f) || input * mSpeed > 0.0f))
	{
		const f32 accelTime = (mMaxSpeed - input * mSpeed) / mAcceleration;
		if (accelTime > remainingTime)
		{
			const f32 deltaSpeed = input * mAcceleration * remainingTime;
			movement += (mSpeed + 0.5f * deltaSpeed) * remainingTime;
			mSpeed += deltaSpeed;
			remainingTime = 0.0f;
		}
		else
		{
			movement += 0.5f * (mSpeed + input * mMaxSpeed) * accelTime;
			mSpeed = input * mMaxSpeed;
			remainingTime -= accelTime;
		}
	}

	movement += mSpeed * remainingTime;

	mPosition += movement;
	if (mPosition > mLength)
	{
		mPosition = mLength;
		mSpeed = 0.0f;
	}
	else if (mPosition < -mLength)
	{
		mPosition = -mLength;
		mSpeed = 0.0f;
	}

	mOwner.Transform.Position = mCenter + mPosition / mLength * mExtent;
}
