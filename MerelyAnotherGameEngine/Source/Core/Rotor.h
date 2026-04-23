#pragma once

#include <glm/vec3.hpp>

#include <cmath>

namespace mage
{
	struct Rotor
	{
		glm::vec3 Rotate(glm::vec3 inVector) const
		{
			const f32 x = S * inVector.x + XY * inVector.y - ZX * inVector.z;
			const f32 y = S * inVector.y + YZ * inVector.z - XY * inVector.x;
			const f32 z = S * inVector.z + ZX * inVector.x - YZ * inVector.y;
			const f32 xyz = XY * inVector.z + YZ * inVector.x + ZX * inVector.y;

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

		static Rotor FromAxisAndAngle(glm::vec3 inAxis, f32 inAngleRad)
		{
			const f32 invSqrt = 1.0f / std::sqrtf(inAxis.x * inAxis.x + inAxis.y * inAxis.y + inAxis.z * inAxis.z);
			const f32 halfAngle = 0.5f * inAngleRad;
			const f32 sine = std::sinf(halfAngle);
			const f32 factor = invSqrt * sine;

			Rotor result;
			result.S = std::cosf(halfAngle);
			result.XY = factor * inAxis.z;
			result.YZ = factor * inAxis.x;
			result.ZX = factor * inAxis.y;
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

		f32 S;
		f32 XY;
		f32 YZ;
		f32 ZX;
	};
}
