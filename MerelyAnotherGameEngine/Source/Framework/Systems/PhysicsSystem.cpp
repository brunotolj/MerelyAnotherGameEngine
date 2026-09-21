#include "Framework/GameWorld.h"
#include "Framework/Systems/PhysicsSystem.h"
#include "Engine/Engine.h"

PhysicsSystem::PhysicsSystem(GameWorld& inWorld) : GameSystemWithPrerequisites(inWorld)
{
	mScene = gEngine->mPhysicsEngine.CreateScene();
}

PhysicsSystem::~PhysicsSystem()
{
	PX_RELEASE(mScene);
}

void PhysicsSystem::Update(f32 inDeltaTime)
{
	for (DynamicRigidBodyEntity* dynamicBody : mWorld.GetEntities<DynamicRigidBodyEntity>())
	{
		if (!dynamicBody->mIsKinematic) continue;
		physx::PxTransform pose = ReadTransformFromTransformTree(dynamicBody->GetParentEntity().mTransformId);
		dynamicBody->mPhysicsActor->setKinematicTarget(pose);
	}

	mScene->simulate(inDeltaTime);
	mScene->fetchResults(true);

	for (DynamicRigidBodyEntity* dynamicBody : mWorld.GetEntities<DynamicRigidBodyEntity>())
	{
		if (dynamicBody->mIsKinematic) continue;
		physx::PxTransform pose = dynamicBody->mPhysicsActor->getGlobalPose();
		WriteTransformToTransformTree(dynamicBody->GetParentEntity().mTransformId, pose);
	}
}

physx::PxRigidStatic* PhysicsSystem::CreateStaticRigidBody(TransformTreeEntryId inTransformId,
	AssetHandle<PhysicsShape> inShape, AssetHandle<PhysicsMaterial> inMaterial)
{
	physx::PxShape* shape = gEngine->mPhysicsEngine.CreateShape(inShape, inMaterial);
	mage_check(shape);

	physx::PxTransform pose = ReadTransformFromTransformTree(inTransformId);

	physx::PxRigidStatic* actor = gEngine->mPhysicsEngine.CreateStaticActor(pose);
	mage_check(actor);

	actor->attachShape(*shape);
	mScene->addActor(*actor);
	shape->release();

	return actor;
}

physx::PxRigidDynamic* PhysicsSystem::CreateDynamicRigidBody(
	TransformTreeEntryId inTransformId,
	AssetHandle<PhysicsShape> inShape,
	AssetHandle<PhysicsMaterial> inMaterial,
	bool inIsKinematic,
	glm::vec3 inLinearVelocity,
	glm::vec3 inAngularVelocity)
{
	physx::PxShape* shape = gEngine->mPhysicsEngine.CreateShape(inShape, inMaterial);
	mage_check(shape);

	physx::PxTransform pose = ReadTransformFromTransformTree(inTransformId);

	physx::PxRigidDynamic* actor = gEngine->mPhysicsEngine.CreateDynamicActor(pose);
	mage_check(actor);

	if (inIsKinematic)
	{
		actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
	}
	else
	{
		actor->setLinearVelocity((physx::PxVec3 const&)inLinearVelocity, false);
		actor->setAngularVelocity((physx::PxVec3 const&)inAngularVelocity, false);
	}

	actor->attachShape(*shape);
	mScene->addActor(*actor);
	shape->release();

	return actor;
}

void PhysicsSystem::RemoveActor(physx::PxRigidActor* inActor)
{
	mage_check(inActor);
	mScene->removeActor(*inActor);
}

physx::PxTransform PhysicsSystem::ReadTransformFromTransformTree(TransformTreeEntryId inTransformId)
{
	mage::Transform transform = Get<TransformTree>().GetGlobalTransform(inTransformId);

	physx::PxTransform result;
	result.p = (physx::PxVec3 const&)(transform.Position);
	result.q.w = transform.Rotation.S;
	result.q.x = -transform.Rotation.YZ;
	result.q.y = -transform.Rotation.ZX;
	result.q.z = -transform.Rotation.XY;

	return result;
}

void PhysicsSystem::WriteTransformToTransformTree(TransformTreeEntryId inTransformId, physx::PxTransform const& inTransform)
{
	mage::Transform transform;
	transform.Position = (glm::vec3 const&)(inTransform.p);
	transform.Rotation.S = inTransform.q.w;
	transform.Rotation.XY = -inTransform.q.z;
	transform.Rotation.YZ = -inTransform.q.x;
	transform.Rotation.ZX = -inTransform.q.y;

	Get<TransformTree>().SetGlobalTransform(inTransformId, transform);
}
