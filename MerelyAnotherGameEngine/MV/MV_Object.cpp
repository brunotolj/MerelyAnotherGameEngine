#include "MV/MV_Object.h"

glm::mat4 MV::Object::Transform::Matrix()
{
	return
	{
		{ glm::cos(mRotation) * mScale.x, -glm::sin(mRotation) * mScale.x, 0.0f, 0.0f },
		{ glm::sin(mRotation) * mScale.y, glm::cos(mRotation) * mScale.y, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
		{ mPosition.x, mPosition.y, 0.0f, 1.0f },
	};
}