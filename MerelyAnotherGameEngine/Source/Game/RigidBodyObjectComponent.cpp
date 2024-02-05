#include "Game/RigidBodyObjectComponent.h"
#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Physics/PhysicsSystem.h"

RigidBodyObjectComponent::RigidBodyObjectComponent(TransformableObject& owner) :
	GameObjectComponent(owner)
{
}

void RigidBodyObjectComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	physx::PxTransform pose;
	pose.p = reinterpret_cast<const physx::PxVec3&>(mOwner.Transform.Position);
	pose.q.w = mOwner.Transform.Rotation.S;
	pose.q.x = -mOwner.Transform.Rotation.YZ;
	pose.q.y = -mOwner.Transform.Rotation.ZX;
	pose.q.z = -mOwner.Transform.Rotation.XY;

	mPhysicsActor = world.GetPhysicsSystem().AddRigidBody(RigidBodyParams, pose, LinearVelocity, AngularVelocity);
}

void RigidBodyObjectComponent::OnOwnerRemovedFromWorld(GameWorld& world)
{
	world.GetPhysicsSystem().RemoveActor(mPhysicsActor);
}

void RigidBodyObjectComponent::UpdatePrePhysics(float deltaTime)
{
	if (RigidBodyParams.Type == PhysicsSystemObjectType::RigidKinematic)
	{
		physx::PxTransform pose;
		pose.p = reinterpret_cast<const physx::PxVec3&>(mOwner.Transform.Position);
		pose.q.w = mOwner.Transform.Rotation.S;
		pose.q.x = -mOwner.Transform.Rotation.YZ;
		pose.q.y = -mOwner.Transform.Rotation.ZX;
		pose.q.z = -mOwner.Transform.Rotation.XY;

		reinterpret_cast<physx::PxRigidDynamic*>(mPhysicsActor)->setKinematicTarget(pose);
	}
}

void RigidBodyObjectComponent::UpdatePostPhysics(float deltaTime)
{
	if (RigidBodyParams.Type == PhysicsSystemObjectType::RigidDynamic)
	{
		const physx::PxTransform pose = mPhysicsActor->getGlobalPose();
		mOwner.Transform.Position = reinterpret_cast<const glm::vec3&>(pose.p);
		mOwner.Transform.Rotation.S = pose.q.w;
		mOwner.Transform.Rotation.XY = -pose.q.z;
		mOwner.Transform.Rotation.YZ = -pose.q.x;
		mOwner.Transform.Rotation.ZX = -pose.q.y;
	}
}
