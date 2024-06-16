#pragma once

#include <cstdint>
#include <memory>

#include <Foundation/PxTransform.h>
#include <Geometry/PxCustomGeometry.h>

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

struct PhysicsRigidBodyParams
{
	PhysicsSystemObjectType Type = PhysicsSystemObjectType::RigidStatic;

	std::shared_ptr<physx::PxCustomGeometry::Callbacks> CustomGeometryCallbacks = nullptr;
	
	std::shared_ptr<physx::PxGeometry> Geometry = nullptr;

	PhysicsSystemMaterialPtr Material = nullptr;
};
