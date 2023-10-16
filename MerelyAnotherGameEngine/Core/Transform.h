#pragma once

#include "Core/Rotor.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <cmath>

namespace mage
{
	struct Transform
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		mage::Rotor Rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		glm::mat4 Matrix() const
		{
			const float s = Rotation.S;
			const float p = Rotation.XY;
			const float q = Rotation.YZ;
			const float r = Rotation.ZX;
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
				Scale.x * glm::vec4{ (b + e) - 1.0f, (h + d) - 1.0f, (g + c) - 1.0f, 0.0f },
				Scale.y * glm::vec4{ (h + a) - 1.0f, (c + f) - 1.0f, (i + e) - 1.0f, 0.0f },
				Scale.z * glm::vec4{ (g + f) - 1.0f, (i + b) - 1.0f, (a + d) - 1.0f, 0.0f },
				glm::vec4{ Position, 1.0f },
			};
		}
	};
}
