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

		float TEMP_InvAspectRatio = 1.0f;
	};
}
