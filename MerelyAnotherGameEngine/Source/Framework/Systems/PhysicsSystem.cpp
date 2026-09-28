#include "Framework/GameWorld.h"
#include "Framework/Systems/PhysicsSystem.h"
#include "Engine/Engine.h"

#include <iostream>

REGISTER_WORLD_COMPONENT_FACTORY_FUNCTION(PhysicsSystem);

PhysicsSystem::PhysicsSystem(GameWorld& inWorld) : GameSystemWithPrerequisites(inWorld)
{
	mDummyMaterial = Factory<PhysicsMaterial>::Impl().Create("Dummy");
	mScene = gEngine->mPhysicsEngine.CreateScene(mCallbacks);
}

PhysicsSystem::~PhysicsSystem()
{
	PX_RELEASE(mScene);
}

void PhysicsSystem::GetEntityCallbackTypes(mage::Array<std::type_index>& outTypes) const
{
	outTypes.AddConstruct(typeid(StaticRigidBodyEntity));
	outTypes.AddConstruct(typeid(DynamicRigidBodyEntity));
	outTypes.AddConstruct(typeid(StaticTriggerVolumeEntity));
}

void PhysicsSystem::OnEntityCreated(GameEntity* inEntity, GameEntitySetup const* inSetup, std::type_index inType)
{
	if (inType == typeid(StaticRigidBodyEntity))
	{
		StaticRigidBodyEntity* entity = (StaticRigidBodyEntity*)(inEntity);
		StaticRigidBodyEntity::Setup const* setup = (StaticRigidBodyEntity::Setup const*)(inSetup);

		entity->mPhysicsActor = CreateStaticRigidBody(entity->GetParentEntity().mTransformId, setup->Shape, setup->Material);
		entity->mPhysicsActor->userData = entity;
	}
	else if (inType == typeid(DynamicRigidBodyEntity))
	{
		DynamicRigidBodyEntity* entity = (DynamicRigidBodyEntity*)(inEntity);
		DynamicRigidBodyEntity::Setup const* setup = (DynamicRigidBodyEntity::Setup const*)(inSetup);

		entity->mPhysicsActor = CreateDynamicRigidBody(entity->GetParentEntity().mTransformId, setup->Shape,
			setup->Material, setup->IsKinematic, setup->LinearVelocity, setup->AngularVelocity);

		entity->mPhysicsActor->userData = entity;
	}
	else if (inType == typeid(StaticTriggerVolumeEntity))
	{
		StaticTriggerVolumeEntity* entity = (StaticTriggerVolumeEntity*)(inEntity);
		StaticTriggerVolumeEntity::Setup const* setup = (StaticTriggerVolumeEntity::Setup const*)(inSetup);

		entity->mPhysicsActor = CreateStaticTriggerVolume(entity->GetParentEntity().mTransformId, setup->Shape);
		entity->mPhysicsActor->userData = entity;
	}
	else
	{
		mage_check(false);
	}
}

void PhysicsSystem::OnEntityDestroyed(GameEntity* inEntity, std::type_index inType)
{
	if (inType == typeid(StaticRigidBodyEntity))
	{
		StaticRigidBodyEntity* entity = (StaticRigidBodyEntity*)(inEntity);

		RemoveActor(entity->mPhysicsActor);
		entity->mPhysicsActor = nullptr;
	}
	else if (inType == typeid(DynamicRigidBodyEntity))
	{
		DynamicRigidBodyEntity* entity = (DynamicRigidBodyEntity*)(inEntity);

		RemoveActor(entity->mPhysicsActor);
		entity->mPhysicsActor = nullptr;
	}
	else if (inType == typeid(StaticTriggerVolumeEntity))
	{
		StaticTriggerVolumeEntity* entity = (StaticTriggerVolumeEntity*)(inEntity);

		RemoveActor(entity->mPhysicsActor);
		entity->mPhysicsActor = nullptr;
	}
	else
	{
		mage_check(false);
	}
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
	
	for (PhysicsCallbacks::OverlapEvent const& overlapEvent : mCallbacks.mTriggerOverlapEvents)
	{
		StaticTriggerVolumeEntity* triggerEntity = (StaticTriggerVolumeEntity*)(overlapEvent.ActorA->userData);
		DynamicRigidBodyEntity* overlappedEntity = (DynamicRigidBodyEntity*)(overlapEvent.ActorB->userData);

		if (overlapEvent.AreActorsOverlapping)
			triggerEntity->mOverlaps.Add(overlappedEntity);
		else
			triggerEntity->mOverlaps.Remove(overlappedEntity);
	}

	mCallbacks.mTriggerOverlapEvents.Empty();
}

physx::PxRigidStatic* PhysicsSystem::CreateStaticRigidBody(TransformTreeEntryId inTransformId, AssetHandle<PhysicsShape> inShape, AssetHandle<PhysicsMaterial> inMaterial)
{
	physx::PxTransform pose = ReadTransformFromTransformTree(inTransformId);

	physx::PxRigidStatic* actor = gEngine->mPhysicsEngine.CreateStaticActor(pose);
	mage_check(actor);

	physx::PxShape* shape = gEngine->mPhysicsEngine.CreateShape(inShape, inMaterial);
	mage_check(shape);

	actor->attachShape(*shape);
	mScene->addActor(*actor);
	shape->release();

	return actor;
}

physx::PxRigidDynamic* PhysicsSystem::CreateDynamicRigidBody(TransformTreeEntryId inTransformId, AssetHandle<PhysicsShape> inShape, AssetHandle<PhysicsMaterial> inMaterial,
	bool inIsKinematic, glm::vec3 inLinearVelocity, glm::vec3 inAngularVelocity)
{
	physx::PxTransform pose = ReadTransformFromTransformTree(inTransformId);

	physx::PxRigidDynamic* actor = gEngine->mPhysicsEngine.CreateDynamicActor(pose);
	mage_check(actor);

	physx::PxShape* shape = gEngine->mPhysicsEngine.CreateShape(inShape, inMaterial);
	mage_check(shape);

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

physx::PxRigidStatic* PhysicsSystem::CreateStaticTriggerVolume(TransformTreeEntryId inTransformId, AssetHandle<PhysicsShape> inShape)
{
	physx::PxTransform pose = ReadTransformFromTransformTree(inTransformId);

	physx::PxRigidStatic* actor = gEngine->mPhysicsEngine.CreateStaticActor(pose);
	mage_check(actor);

	physx::PxShape* shape = gEngine->mPhysicsEngine.CreateShape(inShape, mDummyMaterial);
	mage_check(shape);

	shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
	shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);

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
