#pragma once

#include <cstdint>
#include <memory>

#include <Foundation/PxTransform.h>
#include <Geometry/PxCustomGeometry.h>

enum class PhysicsSystemObjectType : u8
{
	RigidStatic,
	RigidKinematic,
	RigidDynamic
};

struct PhysicsSystemMaterialProperties
{
	f32 StaticFriction;
	f32 DynamicFriction;
	f32 Restitution;
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
