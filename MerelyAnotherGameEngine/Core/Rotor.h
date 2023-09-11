#pragma once

#include <glm/vec3.hpp>

#include <cmath>

namespace mage
{
	struct Rotor
	{
		float S;
		float XY;
		float YZ;
		float ZX;

		static Rotor FromAxisAndAngle(glm::vec3 axis, float angleRad)
		{
			const float invSqrt = 1.0f / std::sqrtf(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
			const float halfAngle = 0.5f * angleRad;
			const float sine = std::sinf(halfAngle);
			const float factor = invSqrt * sine;

			Rotor result;
			result.S = std::cosf(halfAngle);
			result.XY = factor * axis.z;
			result.YZ = factor * axis.x;
			result.ZX = factor * axis.y;
			return result;
		}

		static Rotor Combine(Rotor lhs, Rotor rhs)
		{
			Rotor result;
			result.S = lhs.S * rhs.S - lhs.XY * rhs.XY - lhs.YZ * rhs.YZ - lhs.ZX * rhs.ZX;
			result.XY = lhs.S * rhs.XY + lhs.XY * rhs.S - lhs.YZ * rhs.ZX + lhs.ZX * rhs.YZ;
			result.YZ = lhs.S * rhs.YZ + lhs.XY * rhs.ZX + lhs.YZ * rhs.S - lhs.ZX * rhs.XY;
			result.ZX = lhs.S * rhs.ZX - lhs.XY * rhs.YZ + lhs.YZ * rhs.XY + lhs.ZX * rhs.S;
			return result;
		}
	};
}
