#pragma once

#include "Game/GameObjectComponent.h"
#include "Physics/PhysicsCommon.h"

class TransformableObject;
struct PhysicsSystemMaterial;

namespace physx
{
	class PxRigidActor;
}

class RigidBodyObjectComponent : public GameObjectComponent<TransformableObject>
{
public:
	PhysicsRigidBodyParams RigidBodyParams;

	physx::PxVec3 LinearVelocity = physx::PxVec3(physx::PxZero);

	physx::PxVec3 AngularVelocity = physx::PxVec3(physx::PxZero);

	RigidBodyObjectComponent(TransformableObject& owner);

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) override;

	virtual void OnOwnerRemovedFromWorld(GameWorld& world) override;

	virtual void UpdatePrePhysics(float deltaTime) override;

	virtual void UpdatePostPhysics(float deltaTime) override;

private:
	physx::PxRigidActor* mPhysicsActor = nullptr;
};
