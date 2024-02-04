#pragma once
#include <cstdint>
#include <memory>

enum class PhysicsSystemObjectType : uint8_t
{
	RigidStatic,
	RigidKinematic,
	RigidDynamic
};

struct PhysicsSystemMaterialProperties
{
	float StaticFriction;
	float DynamicFriction;
	float Restitution;
};

struct PhysicsSystemMaterial;
using PhysicsSystemMaterialPtr = std::shared_ptr<PhysicsSystemMaterial>;
