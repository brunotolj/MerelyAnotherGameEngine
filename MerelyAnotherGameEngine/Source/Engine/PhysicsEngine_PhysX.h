#pragma once

#include "Assets/PhysicsMaterial.h"
#include "Assets/PhysicsShape.h"

#include <PxPhysicsAPI.h>

class PhysicsEngine_PhysX : public NonMovable
{
public:
	PhysicsEngine_PhysX();
	~PhysicsEngine_PhysX();

	physx::PxScene* CreateScene() const;
	physx::PxMaterial* CreateMaterial(f32 inStaticFriction, f32 inDynamicFriction, f32 inRestitution) const;
	physx::PxShape* CreateShape(AssetHandle<PhysicsShape> inGeometry, AssetHandle<PhysicsMaterial> inMaterial) const;
	physx::PxRigidStatic* CreateStaticActor(physx::PxTransform const& inPose) const;
	physx::PxRigidDynamic* CreateDynamicActor(physx::PxTransform const& inPose) const;

private:
	physx::PxDefaultAllocator mAllocator;
	physx::PxDefaultErrorCallback mErrorCallback;
	physx::PxFoundation* mFoundation = nullptr;
	physx::PxPhysics* mPhysics = nullptr;
	physx::PxDefaultCpuDispatcher* mDispatcher = nullptr;
};
