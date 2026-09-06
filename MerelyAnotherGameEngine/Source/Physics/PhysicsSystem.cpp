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

void PhysicsSystem::Update(f32 deltaTime)
{
	mScene->simulate(deltaTime);
	mScene->fetchResults(true);
}

physx::PxRigidActor* PhysicsSystem::AddRigidBody(
	const PhysicsRigidBodyParams& params,
	const physx::PxTransform& pose,
	physx::PxVec3 linearVelocity,
	physx::PxVec3 angularVelocity)
{
	physx::PxRigidActor* actor = nullptr;

	physx::PxShape* shape = gEngine->mPhysicsEngine.CreateShape(params.Shape, params.Material);
	mage_check(shape);

	switch (params.Type)
	{
		case PhysicsSystemObjectType::RigidStatic:
		{
			physx::PxRigidStatic* rigidStatic = gEngine->mPhysicsEngine.CreateStaticActor(pose);
			rigidStatic->attachShape(*shape);

			actor = rigidStatic;
			break;
		}

		case PhysicsSystemObjectType::RigidKinematic:
		{
			physx::PxRigidDynamic* rigidDynamic = gEngine->mPhysicsEngine.CreateDynamicActor(pose);
			rigidDynamic->attachShape(*shape);
			rigidDynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);

			actor = rigidDynamic;
			break;
		}

		case PhysicsSystemObjectType::RigidDynamic:
		{
			physx::PxRigidDynamic* rigidDynamic = gEngine->mPhysicsEngine.CreateDynamicActor(pose);
			rigidDynamic->attachShape(*shape);

			rigidDynamic->setLinearVelocity(linearVelocity, false);
			rigidDynamic->setAngularVelocity(angularVelocity, false);

			actor = rigidDynamic;
			break;
		}
	}

	mage_check(actor);

	mScene->addActor(*actor);
	shape->release();

	return actor;
}

void PhysicsSystem::RemoveActor(physx::PxRigidActor* actor)
{
	mage_check(actor);
	mScene->removeActor(*actor);
}
