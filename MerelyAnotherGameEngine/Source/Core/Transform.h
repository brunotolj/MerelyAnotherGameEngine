#pragma once

#include "Core/Rotor.h"

#include <glm/glm.hpp>

#include <cmath>

namespace mage
{
	struct Transform
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		mage::Rotor Rotation = { 1.0f, 0.0f, 0.0f, 0.0f };

		glm::mat4 Matrix() const
		{
			const f32 s = Rotation.S;
			const f32 p = Rotation.XY;
			const f32 q = Rotation.YZ;
			const f32 r = Rotation.ZX;
			f32 a = s + p; a *= a;
			f32 b = s + q; b *= b;
			f32 c = s + r; c *= c;
			f32 d = s - p; d *= d;
			f32 e = s - q; e *= e;
			f32 f = s - r; f *= f;
			f32 g = p + q; g *= g;
			f32 h = q + r; h *= h;
			f32 i = r + p; i *= i;

			return
			{
				glm::vec4{ (b + e) - 1.0f, (h + d) - 1.0f, (g + c) - 1.0f, 0.0f },
				glm::vec4{ (h + a) - 1.0f, (c + f) - 1.0f, (i + e) - 1.0f, 0.0f },
				glm::vec4{ (g + f) - 1.0f, (i + b) - 1.0f, (a + d) - 1.0f, 0.0f },
				glm::vec4{ Position, 1.0f }
			};
		}
	};
}
