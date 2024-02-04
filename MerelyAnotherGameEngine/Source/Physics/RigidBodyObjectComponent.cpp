#include "Physics/RigidBodyObjectComponent.h"
#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Physics/PhysicsSystem.h"

RigidBodyObjectComponent::RigidBodyObjectComponent(TransformableObject& owner) :
	GameObjectComponent(owner)
{
	mPose.p = reinterpret_cast<const physx::PxVec3&>(mOwner.Transform.Position);
	mPose.q.w = mOwner.Transform.Rotation.S;
	mPose.q.x = -mOwner.Transform.Rotation.YZ;
	mPose.q.y = -mOwner.Transform.Rotation.ZX;
	mPose.q.z = -mOwner.Transform.Rotation.XY;
}

void RigidBodyObjectComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	mPhysicsActor = world.GetPhysicsSystem().AddRigidBody(*this);
}

void RigidBodyObjectComponent::OnOwnerRemovedFromWorld(GameWorld& world)
{
	world.GetPhysicsSystem().RemoveActor(mPhysicsActor);
}

void RigidBodyObjectComponent::UpdatePrePhysics(float deltaTime)
{
	if (mType == PhysicsSystemObjectType::RigidKinematic)
	{
		mPose.p = reinterpret_cast<const physx::PxVec3&>(mOwner.Transform.Position);
		mPose.q.w = mOwner.Transform.Rotation.S;
		mPose.q.x = -mOwner.Transform.Rotation.YZ;
		mPose.q.y = -mOwner.Transform.Rotation.ZX;
		mPose.q.z = -mOwner.Transform.Rotation.XY;
		reinterpret_cast<physx::PxRigidDynamic*>(mPhysicsActor)->setKinematicTarget(mPose);
	}
}

void RigidBodyObjectComponent::UpdatePostPhysics(float deltaTime)
{
	if (mType == PhysicsSystemObjectType::RigidDynamic)
	{
		mPose = mPhysicsActor->getGlobalPose();

		mOwner.Transform.Position = reinterpret_cast<const glm::vec3&>(mPose.p);
		mOwner.Transform.Rotation.S = mPose.q.w;
		mOwner.Transform.Rotation.XY = -mPose.q.z;
		mOwner.Transform.Rotation.YZ = -mPose.q.x;
		mOwner.Transform.Rotation.ZX = -mPose.q.y;
	}
}
