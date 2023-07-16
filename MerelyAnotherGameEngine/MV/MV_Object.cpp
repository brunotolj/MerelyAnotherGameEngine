#include "MV/MV_Object.h"

glm::mat4 MV::Object::Transform::Matrix()
{
	return
	{
		{glm::cos(pubRotation) * pubScale.x, -glm::sin(pubRotation) * pubScale.x, 0.0f, 0.0f},
		{glm::sin(pubRotation) * pubScale.y, glm::cos(pubRotation) * pubScale.y, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{pubPosition.x, pubPosition.y, 0.0f, 1.0f},
	};
}