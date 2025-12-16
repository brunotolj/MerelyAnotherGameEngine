#pragma once

#include <glm/vec3.hpp>

#include <cmath>

namespace mage
{
	struct Rotor
	{
		f32 S;
		f32 XY;
		f32 YZ;
		f32 ZX;

		glm::vec3 Rotate(glm::vec3 vector) const
		{
			const f32 x = S * vector.x + XY * vector.y - ZX * vector.z;
			const f32 y = S * vector.y + YZ * vector.z - XY * vector.x;
			const f32 z = S * vector.z + ZX * vector.x - YZ * vector.y;
			const f32 xyz = XY * vector.z + YZ * vector.x + ZX * vector.y;

			return
			{
				x * S + y * XY + xyz * YZ - z * ZX,
				y * S - x * XY + z * YZ + xyz * ZX,
				z * S + xyz * XY - y * YZ + x * ZX
			};
		}

		static Rotor Identity()
		{
			return { 1.0f, 0.0f, 0.0f, 0.0f };
		}

		static Rotor FromAxisAndAngle(glm::vec3 axis, f32 angleRad)
		{
			const f32 invSqrt = 1.0f / std::sqrtf(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
			const f32 halfAngle = 0.5f * angleRad;
			const f32 sine = std::sinf(halfAngle);
			const f32 factor = invSqrt * sine;

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
