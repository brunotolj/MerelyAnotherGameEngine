#include "Physics/PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
{
	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);

	mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale());

	physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, 0.0f, -9.81f);
	mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = mDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	mScene = mPhysics->createScene(sceneDesc);
}

PhysicsSystem::~PhysicsSystem()
{
	PX_RELEASE(mScene);
	PX_RELEASE(mDispatcher);
	PX_RELEASE(mPhysics);
	PX_RELEASE(mFoundation);
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
	physx::PxMaterial* material = params.Material.get() ? &params.Material->Get() : nullptr;

	physx::PxShape* shape = mPhysics->createShape(*params.Geometry, &material, true);
	mage_check(shape);

	switch (params.Type)
	{
		case PhysicsSystemObjectType::RigidStatic:
		{
			physx::PxRigidStatic* rigidStatic = mPhysics->createRigidStatic(pose);
			rigidStatic->attachShape(*shape);

			actor = rigidStatic;
			break;
		}

		case PhysicsSystemObjectType::RigidKinematic:
		{
			physx::PxRigidDynamic* rigidDynamic = mPhysics->createRigidDynamic(pose);
			rigidDynamic->attachShape(*shape);
			rigidDynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);

			actor = rigidDynamic;
			break;
		}

		case PhysicsSystemObjectType::RigidDynamic:
		{
			physx::PxRigidDynamic* rigidDynamic = mPhysics->createRigidDynamic(pose);
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

PhysicsSystemMaterialPtr PhysicsSystem::CreateMaterial(const PhysicsSystemMaterialProperties& props)
{
	physx::PxMaterial* pxMat = mPhysics->createMaterial(
		props.StaticFriction,
		props.DynamicFriction,
		props.Restitution);

	return std::make_shared<PhysicsSystemMaterial>(*this, *pxMat);
}

void PhysicsSystem::RemoveActor(physx::PxRigidActor* actor)
{
	mage_check(actor);
	mScene->removeActor(*actor);
}
