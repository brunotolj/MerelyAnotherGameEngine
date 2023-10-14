#include "MV/MV_Camera.h"

glm::mat4 MV::Camera::GetViewTransform() const
{
	return
	{
		{ TEMP_InvAspectRatio, 0.0f, 0.0f, 0.0f},
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
}