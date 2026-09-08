#pragma once

#include "Framework/GameWorld.h"
#include "Framework/TransformTree.h"
#include "Physics/PhysicsCommon.h"

class RigidBodyObjectComponent;

class PhysicsSystem : public GameSystemWithPrerequisites<TransformTree>
{
public:
	PhysicsSystem(GameWorld& inWorld);
	virtual ~PhysicsSystem();

	virtual void Update(f32 inDeltaTime) override;

	physx::PxRigidActor* AddRigidBody(
		const PhysicsRigidBodyParams& inParams,
		TransformTreeEntryId inTransformId,
		physx::PxVec3 inLinearVelocity,
		physx::PxVec3 inAngularVelocity);

	void RemoveActor(physx::PxRigidActor* inActor);

private:
	struct DynamicActor
	{
		physx::PxRigidDynamic* PhysxActor = nullptr;
		TransformTreeEntryId TransformId = mage::InvalidIndex;

		physx::PxVec3 mLinearVelocity = physx::PxVec3(physx::PxZero);
		physx::PxVec3 mAngularVelocity = physx::PxVec3(physx::PxZero);
	};

	physx::PxTransform ReadTransformFromTransformTree(TransformTreeEntryId inTransformId);
	void WriteTransformToTransformTree(TransformTreeEntryId inTransformId, const physx::PxTransform& inTransform);

	physx::PxScene* mScene = nullptr;

	mage::Array<physx::PxRigidStatic*> mStaticActors;

	mage::Array<DynamicActor> mDynamicActors;
	u32 mKinematicActorCount = 0;
};
