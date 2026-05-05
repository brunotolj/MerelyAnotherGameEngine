#pragma once

#include <glm/glm.hpp>

namespace mage
{
	struct Rotor
	{
		Rotor() : S(1.0f), XY(0.0f), YZ(0.0f), ZX(0.0f) {};

		Rotor(glm::vec3 inAxis, f32 inAngleRad)
		{
			f32 invSqrt = 1.0f / glm::sqrt(inAxis.x * inAxis.x + inAxis.y * inAxis.y + inAxis.z * inAxis.z);
			f32 halfAngle = 0.5f * inAngleRad;
			f32 sine = glm::sin(halfAngle);
			f32 factor = invSqrt * sine;

			S = glm::cos(halfAngle);
			XY = factor * inAxis.z;
			YZ = factor * inAxis.x;
			ZX = factor * inAxis.y;
		}

		glm::vec3 Rotate(glm::vec3 inVector) const
		{
			f32 x = S * inVector.x + XY * inVector.y - ZX * inVector.z;
			f32 y = S * inVector.y + YZ * inVector.z - XY * inVector.x;
			f32 z = S * inVector.z + ZX * inVector.x - YZ * inVector.y;
			f32 xyz = XY * inVector.z + YZ * inVector.x + ZX * inVector.y;

			return
			{
				x * S + xyz * YZ + y * XY - z * ZX,
				y * S + xyz * ZX + z * YZ - x * XY,
				z * S + xyz * XY + x * ZX - y * YZ
			};
		}

		static Rotor Invert(Rotor inRotor)
		{
			Rotor result;
			result.S = inRotor.S;
			result.XY = -inRotor.XY;
			result.YZ = -inRotor.YZ;
			result.ZX = -inRotor.ZX;
			return result;
		}

		static Rotor Combine(Rotor inLhs, Rotor inRhs)
		{
			Rotor result;
			result.S = inLhs.S * inRhs.S - inLhs.XY * inRhs.XY - inLhs.YZ * inRhs.YZ - inLhs.ZX * inRhs.ZX;
			result.XY = inLhs.S * inRhs.XY + inLhs.XY * inRhs.S - inLhs.YZ * inRhs.ZX + inLhs.ZX * inRhs.YZ;
			result.YZ = inLhs.S * inRhs.YZ + inLhs.XY * inRhs.ZX + inLhs.YZ * inRhs.S - inLhs.ZX * inRhs.XY;
			result.ZX = inLhs.S * inRhs.ZX - inLhs.XY * inRhs.YZ + inLhs.YZ * inRhs.XY + inLhs.ZX * inRhs.S;
			return result;
		}

		f32 S;
		f32 XY;
		f32 YZ;
		f32 ZX;
	};

	inline Rotor operator-(Rotor inRotor)
	{
		return Rotor::Invert(inRotor);
	}

	inline Rotor operator*(Rotor inLhs, Rotor inRhs)
	{
		return Rotor::Combine(inLhs, inRhs);
	}
}
