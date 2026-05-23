#pragma once

#include "Physics/PhysicsCommon.h"

class RigidBodyObjectComponent;

class PhysicsSystem : public NonCopyableClass
{
public:
	PhysicsSystem();

	~PhysicsSystem();

	void Update(f32 deltaTime);

	physx::PxRigidActor* AddRigidBody(
		const PhysicsRigidBodyParams& params,
		const physx::PxTransform& pose,
		physx::PxVec3 linearVelocity,
		physx::PxVec3 angularVelocity);

	physx::PxMaterial* CreateMaterial(const PhysicsSystemMaterialProperties& props);

	void RemoveActor(physx::PxRigidActor* actor);

private:
	physx::PxDefaultAllocator mAllocator;
	physx::PxDefaultErrorCallback mErrorCallback;
	physx::PxFoundation* mFoundation = nullptr;
	physx::PxPhysics* mPhysics = nullptr;
	physx::PxDefaultCpuDispatcher* mDispatcher = nullptr;
	physx::PxScene* mScene = nullptr;

	mage::Array<physx::PxMaterial*> mMaterials;
};
