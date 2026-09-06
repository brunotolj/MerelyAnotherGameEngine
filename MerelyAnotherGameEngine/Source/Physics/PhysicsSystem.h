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

	virtual void Update(f32 deltaTime) override;

	physx::PxRigidActor* AddRigidBody(
		const PhysicsRigidBodyParams& params,
		const physx::PxTransform& pose,
		physx::PxVec3 linearVelocity,
		physx::PxVec3 angularVelocity);

	void RemoveActor(physx::PxRigidActor* actor);

private:
	physx::PxScene* mScene = nullptr;
};
