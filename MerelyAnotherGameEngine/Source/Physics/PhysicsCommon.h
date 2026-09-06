#pragma once

#include "Assets/PhysicsMaterial.h"
#include "Assets/PhysicsShape.h"

enum class PhysicsSystemObjectType : u8
{
	RigidStatic,
	RigidKinematic,
	RigidDynamic
};

struct PhysicsRigidBodyParams
{
	PhysicsSystemObjectType Type = PhysicsSystemObjectType::RigidStatic;
	AssetHandle<PhysicsShape> Shape;
	AssetHandle<PhysicsMaterial> Material;
};
