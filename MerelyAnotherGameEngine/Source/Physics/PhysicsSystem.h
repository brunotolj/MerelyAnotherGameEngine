#pragma once

#include "Core/NonCopyable.h"
#include "Physics/PhysicsCommon.h"

#include <PxPhysicsAPI.h>

class RigidBodyObjectComponent;

class PhysicsSystem : public NonCopyableClass
{
public:
	PhysicsSystem();

	~PhysicsSystem();

	void Update(float deltaTime);

	physx::PxRigidActor* AddRigidBody(RigidBodyObjectComponent& object);

	PhysicsSystemMaterialPtr CreateMaterial(const PhysicsSystemMaterialProperties& props);

	void RemoveActor(physx::PxRigidActor* actor);

private:
	physx::PxDefaultAllocator mAllocator;
	physx::PxDefaultErrorCallback mErrorCallback;
	physx::PxFoundation* mFoundation = nullptr;
	physx::PxPhysics* mPhysics = nullptr;
	physx::PxDefaultCpuDispatcher* mDispatcher = nullptr;
	physx::PxScene* mScene = nullptr;
	physx::PxPvd* mPvd = nullptr;
};

struct PhysicsSystemMaterial : public NonCopyableStruct
{
	PhysicsSystemMaterial(PhysicsSystem& system, physx::PxMaterial& material) :
		mSystem(system), mMaterial(material) {}

	~PhysicsSystemMaterial() { mMaterial.release(); }

	physx::PxMaterial& Get() { return mMaterial; }

private:
	PhysicsSystem& mSystem;
	physx::PxMaterial& mMaterial;
};
