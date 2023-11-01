#include "Rendering/MV_TransformComponent.h"

glm::mat4 MV::TransformComponent::NormalMatrix() const
{
	const float s = mTransform.Rotation.S;
	const float p = mTransform.Rotation.XY;
	const float q = mTransform.Rotation.YZ;
	const float r = mTransform.Rotation.ZX;
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
		glm::vec4{ (b + e) - 1.0f, (h + d) - 1.0f, (g + c) - 1.0f, 0.0f } / mTransform.Scale.x,
		glm::vec4{ (h + a) - 1.0f, (c + f) - 1.0f, (i + e) - 1.0f, 0.0f } / mTransform.Scale.y,
		glm::vec4{ (g + f) - 1.0f, (i + b) - 1.0f, (a + d) - 1.0f, 0.0f } / mTransform.Scale.z,
		glm::vec4(0.0f),
	};
}
