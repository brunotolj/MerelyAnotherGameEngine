#include "Core/Asserts.h"
#include "MV/MV_Camera.h"

glm::mat4 MV::Camera::GetViewTransform() const
{
	return mPerspectiveTransform;
}

void MV::Camera::SetPerspectiveParams(float nearPlane, float farPlane, float verticalFOV, float aspectRatio)
{
	check(nearPlane >= 0.0f && farPlane > nearPlane);
	check(verticalFOV > 0.0f && glm::degrees(verticalFOV) < 180.0f);
	check(aspectRatio > 0.0f);

	const float fovFactor = 1.0f / glm::tan(verticalFOV / 2.0f);
	const float planeDelta = farPlane - nearPlane;

	mPerspectiveTransform = 
	{
		{ fovFactor / aspectRatio, 0.0f, 0.0f, 0.0f},
		{ 0.0f, fovFactor, 0.0f, 0.0f },
		{ 0.0f, 0.0f, farPlane / planeDelta, 1.0f },
		{ 0.0f, 0.0f, -farPlane * nearPlane / planeDelta, 0.0f}
		//{ 0.0f, 0.0f, farPlane / planeDelta, -farPlane * nearPlane / planeDelta },
//		{ 0.0f, 0.0f, 1.0f, 0.0f}
	};
}