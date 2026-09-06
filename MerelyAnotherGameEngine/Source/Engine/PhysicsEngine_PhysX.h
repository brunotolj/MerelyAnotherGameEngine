#pragma once

#include "Assets/PhysicsMaterial.h"
#include "Assets/PhysicsShape.h"

#include <PxPhysicsAPI.h>

class PhysicsEngine_PhysX : public NonMovable
{
public:
	struct MaterialProperties
	{
		f32 StaticFriction;
		f32 DynamicFriction;
		f32 Restitution;
	};

	enum class ActorType : u8
	{
		RigidStatic,
		RigidKinematic,
		RigidDynamic
	};

	PhysicsEngine_PhysX();
	~PhysicsEngine_PhysX();

	physx::PxScene* CreateScene() const;
	physx::PxMaterial* CreateMaterial(MaterialProperties inProperties) const;
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
