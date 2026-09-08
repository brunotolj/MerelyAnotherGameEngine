#include "Physics/PhysicsSystem.h"
#include "Framework/GameWorld.h"
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
	for (u32 i = 0; i < mKinematicActorCount; ++i)
	{
		const physx::PxTransform pose = ReadTransformFromTransformTree(mDynamicActors[i].TransformId);
		mDynamicActors[i].PhysxActor->setKinematicTarget(pose);
	}

	mScene->simulate(inDeltaTime);
	mScene->fetchResults(true);

	for (u32 i = 0; i < mKinematicActorCount; ++i)
	{
		mDynamicActors[i].mLinearVelocity = mDynamicActors[i].PhysxActor->getLinearVelocity();
		mDynamicActors[i].mAngularVelocity = mDynamicActors[i].PhysxActor->getAngularVelocity();
	}

	for (u32 i = mKinematicActorCount; i < mDynamicActors.GetSize(); ++i)
	{
		mDynamicActors[i].mLinearVelocity = mDynamicActors[i].PhysxActor->getLinearVelocity();
		mDynamicActors[i].mAngularVelocity = mDynamicActors[i].PhysxActor->getAngularVelocity();

		const physx::PxTransform pose = mDynamicActors[i].PhysxActor->getGlobalPose();
		WriteTransformToTransformTree(mDynamicActors[i].TransformId, pose);
	}
}

physx::PxRigidActor* PhysicsSystem::AddRigidBody(
	const PhysicsRigidBodyParams& inParams,
	TransformTreeEntryId inTransformId,
	physx::PxVec3 inLinearVelocity,
	physx::PxVec3 inAngularVelocity)
{
	physx::PxRigidActor* actor = nullptr;

	physx::PxShape* shape = gEngine->mPhysicsEngine.CreateShape(inParams.Shape, inParams.Material);
	mage_check(shape);

	physx::PxTransform pose = ReadTransformFromTransformTree(inTransformId);

	switch (inParams.Type)
	{
		case PhysicsSystemObjectType::RigidStatic:
		{
			physx::PxRigidStatic* rigidStatic = gEngine->mPhysicsEngine.CreateStaticActor(pose);
			rigidStatic->attachShape(*shape);

			mStaticActors.Add(rigidStatic);
			actor = rigidStatic;

			break;
		}

		case PhysicsSystemObjectType::RigidKinematic:
		{
			physx::PxRigidDynamic* rigidDynamic = gEngine->mPhysicsEngine.CreateDynamicActor(pose);
			rigidDynamic->attachShape(*shape);
			rigidDynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);

			mDynamicActors.InsertSwap({ rigidDynamic, inTransformId, inLinearVelocity, inAngularVelocity }, mKinematicActorCount++);
			actor = rigidDynamic;

			break;
		}

		case PhysicsSystemObjectType::RigidDynamic:
		{
			physx::PxRigidDynamic* rigidDynamic = gEngine->mPhysicsEngine.CreateDynamicActor(pose);
			rigidDynamic->attachShape(*shape);

			rigidDynamic->setLinearVelocity(inLinearVelocity, false);
			rigidDynamic->setAngularVelocity(inAngularVelocity, false);

			mDynamicActors.Add({ rigidDynamic, inTransformId, inLinearVelocity, inAngularVelocity });
			actor = rigidDynamic;

			break;
		}
	}

	mage_check(actor);

	mScene->addActor(*actor);
	shape->release();

	return actor;
}

void PhysicsSystem::RemoveActor(physx::PxRigidActor* inActor)
{
	mage_check(inActor);

	mStaticActors.Remove(reinterpret_cast<physx::PxRigidStatic*>(inActor));

	u32 index = mDynamicActors.GetIndex([inActor](const DynamicActor& element)
	{
		return element.PhysxActor == reinterpret_cast<physx::PxRigidDynamic*>(inActor);
	});

	if (index < mKinematicActorCount)
	{
		mDynamicActors.Swap(index, --mKinematicActorCount);
		mDynamicActors.RemoveAtSwap(mKinematicActorCount);
	}
	else if (index < mage::InvalidIndex)
	{
		mDynamicActors.RemoveAtSwap(index);
	}

	mScene->removeActor(*inActor);
}

physx::PxTransform PhysicsSystem::ReadTransformFromTransformTree(TransformTreeEntryId inTransformId)
{
	mage::Transform transform = Get<TransformTree>().GetGlobalTransform(inTransformId);

	physx::PxTransform result;
	result.p = reinterpret_cast<const physx::PxVec3&>(transform.Position);
	result.q.w = transform.Rotation.S;
	result.q.x = -transform.Rotation.YZ;
	result.q.y = -transform.Rotation.ZX;
	result.q.z = -transform.Rotation.XY;

	return result;
}

void PhysicsSystem::WriteTransformToTransformTree(TransformTreeEntryId inTransformId, const physx::PxTransform& inTransform)
{
	mage::Transform transform;
	transform.Position = reinterpret_cast<const glm::vec3&>(inTransform.p);
	transform.Rotation.S = inTransform.q.w;
	transform.Rotation.XY = -inTransform.q.z;
	transform.Rotation.YZ = -inTransform.q.x;
	transform.Rotation.ZX = -inTransform.q.y;

	Get<TransformTree>().SetGlobalTransform(inTransformId, transform);
}
