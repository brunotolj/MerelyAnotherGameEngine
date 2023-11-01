#include "Core/Asserts.h"
#include "Rendering/MV_Camera.h"

glm::mat4 MV::Camera::GetViewTransform() const
{
	return glm::inverse(mTransformComponent.mTransform.Matrix());
}

const glm::mat4& MV::Camera::GetProjectionTransform() const
{
	return mPerspectiveTransform;
}

void MV::Camera::SetPerspectiveParams(float nearPlane, float farPlane, float horizontalFOV, float aspectRatio)
{
	mage_check(nearPlane >= 0.0f && farPlane > nearPlane);
	mage_check(horizontalFOV > 0.0f && glm::degrees(horizontalFOV) < 180.0f);
	mage_check(aspectRatio > 0.0f);

	const float fovFactor = 1.0f / glm::tan(horizontalFOV / 2.0f);
	const float planeDelta = farPlane - nearPlane;

	mPerspectiveTransform =
	{
		{ fovFactor, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, farPlane / planeDelta, 1.0f },
		{ 0.0f, -fovFactor * aspectRatio, 0.0f, 0.0f },
		{ 0.0f, 0.0f, -farPlane * nearPlane / planeDelta, 0.0f }
	};
}