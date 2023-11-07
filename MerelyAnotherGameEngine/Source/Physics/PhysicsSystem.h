#pragma once

#include "Core/NonCopyable.h"
#include "Physics/PhysicsCommon.h"

#include <map>

#include <PxPhysicsAPI.h>

class RigidBodyObjectComponent;

class PhysicsSystem : public NonCopyableClass
{
public:
	PhysicsSystem();

	~PhysicsSystem();

	void Update(float deltaTime);

	physx::PxRigidActor* AddRigidBody(RigidBodyObjectComponent& object);

	void RemoveActor(physx::PxRigidActor* actor);

private:
	physx::PxDefaultAllocator mAllocator;
	physx::PxDefaultErrorCallback mErrorCallback;
	physx::PxFoundation* mFoundation = nullptr;
	physx::PxPhysics* mPhysics = nullptr;
	physx::PxDefaultCpuDispatcher* mDispatcher = nullptr;
	physx::PxScene* mScene = nullptr;
	physx::PxMaterial* mDefaultMaterial = nullptr;
	physx::PxPvd* mPvd = nullptr;
};
