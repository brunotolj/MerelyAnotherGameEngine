#include "Game/RigidBodyObjectComponent.h"
#include "Framework/GameWorld.h"
#include "Physics/PhysicsSystem.h"

RigidBodyObjectComponent::RigidBodyObjectComponent(TransformableObject& owner, const ComponentTemplate<RigidBodyObjectComponent>& creationTemplate) :
	GameObjectComponent(owner),
	mRigidBodyParams(creationTemplate.RigidBodyParams),
	mLinearVelocity(creationTemplate.InitialLinearVelocity),
	mAngularVelocity(creationTemplate.InitialAngularVelocity)
{
}

void RigidBodyObjectComponent::OnOwnerAddedToWorld()
{
	mage::Transform transform = mOwner.GetTransform();
	physx::PxTransform pose;
	pose.p = reinterpret_cast<const physx::PxVec3&>(transform.Position);
	pose.q.w = transform.Rotation.S;
	pose.q.x = -transform.Rotation.YZ;
	pose.q.y = -transform.Rotation.ZX;
	pose.q.z = -transform.Rotation.XY;

	mPhysicsActor = mOwner.mWorld.GetComponent<PhysicsSystem>()->AddRigidBody(mRigidBodyParams, pose, mLinearVelocity, mAngularVelocity);
}

void RigidBodyObjectComponent::OnOwnerRemovedFromWorld()
{
	mOwner.mWorld.GetComponent<PhysicsSystem>()->RemoveActor(mPhysicsActor);
}

void RigidBodyObjectComponent::UpdatePrePhysics(f32 deltaTime)
{
	if (mRigidBodyParams.Type == PhysicsSystemObjectType::RigidKinematic)
	{
		mage::Transform transform = mOwner.GetTransform();
		physx::PxTransform pose;
		pose.p = reinterpret_cast<const physx::PxVec3&>(transform.Position);
		pose.q.w = transform.Rotation.S;
		pose.q.x = -transform.Rotation.YZ;
		pose.q.y = -transform.Rotation.ZX;
		pose.q.z = -transform.Rotation.XY;

		reinterpret_cast<physx::PxRigidDynamic*>(mPhysicsActor)->setKinematicTarget(pose);
	}
}

void RigidBodyObjectComponent::UpdatePostPhysics(f32 deltaTime)
{
	if (mRigidBodyParams.Type == PhysicsSystemObjectType::RigidDynamic)
	{
		mage::Transform transform;
		const physx::PxTransform pose = mPhysicsActor->getGlobalPose();
		transform.Position = reinterpret_cast<const glm::vec3&>(pose.p);
		transform.Rotation.S = pose.q.w;
		transform.Rotation.XY = -pose.q.z;
		transform.Rotation.YZ = -pose.q.x;
		transform.Rotation.ZX = -pose.q.y;
		mOwner.SetTransform(transform);
	}

	if (mRigidBodyParams.Type != PhysicsSystemObjectType::RigidStatic)
	{
		mLinearVelocity = reinterpret_cast<physx::PxRigidDynamic*>(mPhysicsActor)->getLinearVelocity();
		mAngularVelocity = reinterpret_cast<physx::PxRigidDynamic*>(mPhysicsActor)->getAngularVelocity();
	}
}
