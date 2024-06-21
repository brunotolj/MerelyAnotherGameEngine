#include "Core/Asserts.h"
#include "Game/CameraComponent.h"
#include "Game/GameWorld.h"
#include "Rendering/RenderSystem.h"

CameraComponent::CameraComponent(TransformableObject& owner, const ComponentTemplate<CameraComponent>& creationTemplate) :
	GameObjectComponent(owner),
	mNearPlane(creationTemplate.NearPlane),
	mFarPlane(creationTemplate.FarPlane),
	mHorizontalFOV(creationTemplate.HorizontalFOV)
{
	mage_check(mNearPlane >= 0.0f && mFarPlane > mNearPlane);
	mage_check(mHorizontalFOV > 0.0f && glm::degrees(mHorizontalFOV) < 180.0f);
	CachePerspectiveTransform();
}

glm::mat4 CameraComponent::GetViewTransform() const
{
	return glm::inverse(mOwner.Transform.Matrix());
}

const glm::mat4& CameraComponent::GetProjectionTransform() const
{
	return mPerspectiveTransform;
}

void CameraComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	world.GetRenderSystem().SetCamera(this);
}

void CameraComponent::OnOwnerRemovedFromWorld(GameWorld& world)
{
	world.GetRenderSystem().SetCamera(nullptr);
}

void CameraComponent::UpdatePostPhysics(float deltaTime)
{
	RenderSystem& renderSystem = mOwner.GetWorld()->GetRenderSystem();
	const float newAspectRatio = renderSystem.GetAspectRatio();

	if (newAspectRatio != mAspectRatio)
	{
		mAspectRatio = newAspectRatio;
		CachePerspectiveTransform();
	}
}

void CameraComponent::CachePerspectiveTransform()
{
	const float fovFactor = 1.0f / glm::tan(mHorizontalFOV / 2.0f);
	const float planeDelta = mFarPlane - mNearPlane;

	mPerspectiveTransform =
	{
		{ fovFactor, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, mFarPlane / planeDelta, 1.0f },
		{ 0.0f, -fovFactor * mAspectRatio, 0.0f, 0.0f },
		{ 0.0f, 0.0f, -mFarPlane * mNearPlane / planeDelta, 0.0f }
	};
}
