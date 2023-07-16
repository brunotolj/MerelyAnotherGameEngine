#include "MV/MV_Object.h"

glm::mat4 MV::Object::Transform::Matrix()
{
	return
	{
		{glm::cos(Rotation) * Scale.x, -glm::sin(Rotation) * Scale.x, 0.0f, 0.0f},
		{glm::sin(Rotation) * Scale.y, glm::cos(Rotation) * Scale.y, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{Position.x, Position.y, 0.0f, 1.0f},
	};
}