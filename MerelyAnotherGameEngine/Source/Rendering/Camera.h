#pragma once

#include "Core/NonCopyable.h"
#include "Core/Transform.h"

class Camera : public NonCopyableClass
{
public:
	mage::Transform mTransform;

	glm::mat4 GetViewTransform() const;

	const glm::mat4& GetProjectionTransform() const;

	void SetPerspectiveParams(float near, float far, float verticalFOV, float aspectRatio);

private:
	glm::mat4 mPerspectiveTransform;
};
