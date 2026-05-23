#pragma once

#include "Physics/PhysicsGeometry.h"

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

struct PhysicsRigidBodyParams
{
	PhysicsSystemObjectType Type = PhysicsSystemObjectType::RigidStatic;

	PhysicsGeometry Geometry;

	physx::PxMaterial* Material = nullptr;
};
