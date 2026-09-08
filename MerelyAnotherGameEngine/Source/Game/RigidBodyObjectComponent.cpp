#include "Game/RigidBodyObjectComponent.h"
#include "Framework/GameWorld.h"
#include "Physics/PhysicsSystem.h"

RigidBodyObjectComponent::RigidBodyObjectComponent(TransformableObject& owner, const ComponentTemplate<RigidBodyObjectComponent>& creationTemplate) :
	GameObjectComponent(owner),
	mRigidBodyParams(creationTemplate.RigidBodyParams),
	mInitialLinearVelocity(creationTemplate.InitialLinearVelocity),
	mInitialAngularVelocity(creationTemplate.InitialAngularVelocity)
{
}

void RigidBodyObjectComponent::OnOwnerAddedToWorld()
{
	mPhysicsActor = mOwner.mWorld.GetComponent<PhysicsSystem>()->AddRigidBody(
		mRigidBodyParams, mOwner.GetTransformId(), mInitialLinearVelocity, mInitialAngularVelocity);
}

void RigidBodyObjectComponent::OnOwnerRemovedFromWorld()
{
	mOwner.mWorld.GetComponent<PhysicsSystem>()->RemoveActor(mPhysicsActor);
}
