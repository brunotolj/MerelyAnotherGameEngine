#pragma once

#include "Core/NonCopyable.h"
#include "MV/MV_TransformComponent.h"

namespace MV
{
	class Camera : public NonCopyableClass
	{
	public:
		TransformComponent mTransformComponent;

		glm::mat4 GetViewTransform() const;

		const glm::mat4& GetProjectionTransform() const;

		void SetPerspectiveParams(float near, float far, float verticalFOV, float aspectRatio);

	private:
		glm::mat4 mPerspectiveTransform;
	};
}
