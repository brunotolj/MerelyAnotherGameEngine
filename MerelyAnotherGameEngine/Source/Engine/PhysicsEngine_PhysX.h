#pragma once

#include "Assets/PhysicsMaterial.h"
#include "Assets/PhysicsShape.h"

#include <PxPhysicsAPI.h>

class PhysicsCallbacks : public physx::PxSimulationEventCallback
{
public:
	struct OverlapEvent
	{
		physx::PxActor* ActorA = nullptr;
		physx::PxActor* ActorB = nullptr;
		bool AreActorsOverlapping = false;
	};

	virtual void onConstraintBreak(physx::PxConstraintInfo* inConstraints, physx::PxU32 inCount) override {}
	virtual void onWake(physx::PxActor** inActors, physx::PxU32 inCount) override {}
	virtual void onSleep(physx::PxActor** inActors, physx::PxU32 inCount) override {}
	virtual void onContact(physx::PxContactPairHeader const& inPairHeader, physx::PxContactPair const* inPairs, physx::PxU32 inNbPairs) override {}
	virtual void onTrigger(physx::PxTriggerPair* inPairs, physx::PxU32 inCount) override;
	virtual void onAdvance(physx::PxRigidBody const* const* inBodyBuffer, physx::PxTransform const* inPoseBuffer, physx::PxU32 inCount) override {}

	mage::Array<OverlapEvent> mTriggerOverlapEvents;
};

class PhysicsEngine_PhysX : public NonMovable
{
public:
	PhysicsEngine_PhysX();
	~PhysicsEngine_PhysX();

	physx::PxScene* CreateScene(PhysicsCallbacks& inCallbacks) const;
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
