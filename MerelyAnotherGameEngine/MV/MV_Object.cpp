#include "MV/MV_Object.h"

glm::mat4 MV::Object::Transform::Matrix()
{
	const float s = mRotation.S;
	const float p = mRotation.XY;
	const float q = mRotation.YZ;
	const float r = mRotation.ZX;
	float a = s + p; a *= a;
	float b = s + q; b *= b;
	float c = s + r; c *= c;
	float d = s - p; d *= d;
	float e = s - q; e *= e;
	float f = s - r; f *= f;
	float g = p + q; g *= g;
	float h = q + r; h *= h;
	float i = r + p; i *= i;

	return
	{
		mScale.x * glm::vec4{ (b + e) - 1.0f, (h + d) - 1.0f, (g + c) - 1.0f, 0.0f },
		mScale.y * glm::vec4{ (h + a) - 1.0f, (c + f) - 1.0f, (i + e) - 1.0f, 0.0f },
		mScale.z * glm::vec4{ (g + f) - 1.0f, (i + b) - 1.0f, (a + d) - 1.0f, 0.0f },
		glm::vec4{ mPosition, 1.0f },
	};
}
