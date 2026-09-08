#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"
#include "Physics/PhysicsCommon.h"

struct PhysicsSystemMaterial;

namespace physx
{
	class PxRigidActor;
}

template<>
struct ComponentTemplate<class RigidBodyObjectComponent>
{
	PhysicsRigidBodyParams RigidBodyParams;

	physx::PxVec3 InitialLinearVelocity = physx::PxVec3(physx::PxZero);

	physx::PxVec3 InitialAngularVelocity = physx::PxVec3(physx::PxZero);
};

class RigidBodyObjectComponent : public GameObjectComponent<TransformableObject>
{
public:
	RigidBodyObjectComponent(TransformableObject& owner, const ComponentTemplate<RigidBodyObjectComponent>& creationTemplate);

protected:
	virtual void OnOwnerAddedToWorld() override final;
	virtual void OnOwnerRemovedFromWorld() override final;

private:
	PhysicsRigidBodyParams mRigidBodyParams;

	physx::PxVec3 mInitialLinearVelocity;
	physx::PxVec3 mInitialAngularVelocity;

	physx::PxRigidActor* mPhysicsActor = nullptr;
};
