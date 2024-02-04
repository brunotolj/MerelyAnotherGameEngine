#include "Physics/RigidBodyObjectComponent.h"
#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Physics/PhysicsSystem.h"

RigidBodyObjectComponent::RigidBodyObjectComponent(GameObject& owner) :
	GameObjectComponent(owner)
{
	mPose.p = reinterpret_cast<const physx::PxVec3&>(mOwner.mTransform.Position);
	mPose.q.w = mOwner.mTransform.Rotation.S;
	mPose.q.x = -mOwner.mTransform.Rotation.YZ;
	mPose.q.y = -mOwner.mTransform.Rotation.ZX;
	mPose.q.z = -mOwner.mTransform.Rotation.XY;
}

void RigidBodyObjectComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	mPhysicsActor = world.mPhysicsSystem->AddRigidBody(*this);
}

void RigidBodyObjectComponent::OnOwnerRemovedFromWorld(GameWorld& world)
{
	world.mPhysicsSystem->RemoveActor(mPhysicsActor);
}

void RigidBodyObjectComponent::UpdatePrePhysics(float deltaTime)
{
	if (mType == PhysicsSystemObjectType::RigidKinematic)
	{
		mPose.p = reinterpret_cast<const physx::PxVec3&>(mOwner.mTransform.Position);
		mPose.q.w = mOwner.mTransform.Rotation.S;
		mPose.q.x = -mOwner.mTransform.Rotation.YZ;
		mPose.q.y = -mOwner.mTransform.Rotation.ZX;
		mPose.q.z = -mOwner.mTransform.Rotation.XY;
		reinterpret_cast<physx::PxRigidDynamic*>(mPhysicsActor)->setKinematicTarget(mPose);
	}
}

void RigidBodyObjectComponent::UpdatePostPhysics(float deltaTime)
{
	if (mType == PhysicsSystemObjectType::RigidDynamic)
	{
		mPose = mPhysicsActor->getGlobalPose();

		mOwner.mTransform.Position = reinterpret_cast<const glm::vec3&>(mPose.p);
		mOwner.mTransform.Rotation.S = mPose.q.w;
		mOwner.mTransform.Rotation.XY = -mPose.q.z;
		mOwner.mTransform.Rotation.YZ = -mPose.q.x;
		mOwner.mTransform.Rotation.ZX = -mPose.q.y;
	}
}
