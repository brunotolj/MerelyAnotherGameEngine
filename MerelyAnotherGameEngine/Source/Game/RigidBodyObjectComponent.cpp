#include "Game/RigidBodyObjectComponent.h"
#include "Game/GameWorld.h"
#include "Physics/PhysicsSystem.h"

RigidBodyObjectComponent::RigidBodyObjectComponent(TransformableObject& owner, const ComponentTemplate<RigidBodyObjectComponent>& creationTemplate) :
	GameObjectComponent(owner),
	mRigidBodyParams(creationTemplate.RigidBodyParams),
	mLinearVelocity(creationTemplate.InitialLinearVelocity),
	mAngularVelocity(creationTemplate.InitialAngularVelocity)
{
}

void RigidBodyObjectComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	physx::PxTransform pose;
	pose.p = reinterpret_cast<const physx::PxVec3&>(mOwner.mTransform.Position);
	pose.q.w = mOwner.mTransform.Rotation.S;
	pose.q.x = -mOwner.mTransform.Rotation.YZ;
	pose.q.y = -mOwner.mTransform.Rotation.ZX;
	pose.q.z = -mOwner.mTransform.Rotation.XY;

	mPhysicsActor = world.mPhysicsSystem.AddRigidBody(mRigidBodyParams, pose, mLinearVelocity, mAngularVelocity);
}

void RigidBodyObjectComponent::OnOwnerRemovedFromWorld(GameWorld& world)
{
	world.mPhysicsSystem.RemoveActor(mPhysicsActor);
}

void RigidBodyObjectComponent::UpdatePrePhysics(f32 deltaTime)
{
	if (mRigidBodyParams.Type == PhysicsSystemObjectType::RigidKinematic)
	{
		physx::PxTransform pose;
		pose.p = reinterpret_cast<const physx::PxVec3&>(mOwner.mTransform.Position);
		pose.q.w = mOwner.mTransform.Rotation.S;
		pose.q.x = -mOwner.mTransform.Rotation.YZ;
		pose.q.y = -mOwner.mTransform.Rotation.ZX;
		pose.q.z = -mOwner.mTransform.Rotation.XY;

		reinterpret_cast<physx::PxRigidDynamic*>(mPhysicsActor)->setKinematicTarget(pose);
	}
}

void RigidBodyObjectComponent::UpdatePostPhysics(f32 deltaTime)
{
	if (mRigidBodyParams.Type == PhysicsSystemObjectType::RigidDynamic)
	{
		const physx::PxTransform pose = mPhysicsActor->getGlobalPose();
		mOwner.mTransform.Position = reinterpret_cast<const glm::vec3&>(pose.p);
		mOwner.mTransform.Rotation.S = pose.q.w;
		mOwner.mTransform.Rotation.XY = -pose.q.z;
		mOwner.mTransform.Rotation.YZ = -pose.q.x;
		mOwner.mTransform.Rotation.ZX = -pose.q.y;
	}

	if (mRigidBodyParams.Type != PhysicsSystemObjectType::RigidStatic)
	{
		mLinearVelocity = reinterpret_cast<physx::PxRigidDynamic*>(mPhysicsActor)->getLinearVelocity();
		mAngularVelocity = reinterpret_cast<physx::PxRigidDynamic*>(mPhysicsActor)->getAngularVelocity();
	}
}
