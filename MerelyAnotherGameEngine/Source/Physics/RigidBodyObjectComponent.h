#pragma once

#include "Game/GameObjectComponent.h"
#include "Physics/PhysicsCommon.h"

#include <memory>

#include <Foundation/PxTransform.h>
#include <Geometry/PxCustomGeometry.h>

namespace physx
{
	class PxRigidActor;
}

class RigidBodyObjectComponent : public GameObjectComponent
{
public:
	std::unique_ptr<physx::PxGeometry> mGeometry = nullptr;

	std::unique_ptr<physx::PxCustomGeometry::Callbacks> mCustomGeometryCallbacks = nullptr;

	PhysicsSystemObjectType mType = PhysicsSystemObjectType::RigidStatic;

	physx::PxTransform mPose = physx::PxTransform(physx::PxIdentity);
	
	physx::PxVec3 mLinearVelocity = physx::PxVec3(physx::PxZero);

	physx::PxVec3 mAngularVelocity = physx::PxVec3(physx::PxZero);

	RigidBodyObjectComponent(GameObject& owner);

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) override;

	virtual void OnOwnerRemovedFromWorld(GameWorld& world) override;

	virtual void UpdatePrePhysics(float deltaTime) override;

	virtual void UpdatePostPhysics(float deltaTime) override;

private:
	physx::PxRigidActor* mPhysicsActor = nullptr;
};
