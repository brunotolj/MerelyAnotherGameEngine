#include "Core/Asserts.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/RigidBodyObjectComponent.h"

PhysicsSystem::PhysicsSystem()
{
	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);

	mPvd = physx::PxCreatePvd(*mFoundation);
	physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

	mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale(), true, mPvd);

	physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, 0.0f, -9.81f);
	mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = mDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	mScene = mPhysics->createScene(sceneDesc);

	physx::PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
}

PhysicsSystem::~PhysicsSystem()
{
	PX_RELEASE(mScene);
	PX_RELEASE(mDispatcher);
	PX_RELEASE(mPhysics);

	if (mPvd)
	{
		physx::PxPvdTransport* transport = mPvd->getTransport();
		mPvd->release();
		mPvd = nullptr;
		PX_RELEASE(transport);
	}

	PX_RELEASE(mFoundation);
}

void PhysicsSystem::Update(float deltaTime)
{
	mScene->simulate(deltaTime);
	mScene->fetchResults(true);
}

physx::PxRigidActor* PhysicsSystem::AddRigidBody(RigidBodyObjectComponent& object)
{
	physx::PxRigidActor* actor = nullptr;
	physx::PxMaterial* material = object.mMaterial.get() ? &object.mMaterial->Get() : nullptr;

	physx::PxShape* shape = mPhysics->createShape(*object.mGeometry, &material, true);
	mage_check(shape);

	switch (object.mType)
	{
		case PhysicsSystemObjectType::RigidStatic:
		{
			physx::PxRigidStatic* rigidStatic = mPhysics->createRigidStatic(object.mPose);
			rigidStatic->attachShape(*shape);

			actor = rigidStatic;
			break;
		}

		case PhysicsSystemObjectType::RigidKinematic:
		{
			physx::PxRigidDynamic* rigidDynamic = mPhysics->createRigidDynamic(object.mPose);
			rigidDynamic->attachShape(*shape);
			rigidDynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);

			actor = rigidDynamic;
			break;
		}

		case PhysicsSystemObjectType::RigidDynamic:
		{
			physx::PxRigidDynamic* rigidDynamic = mPhysics->createRigidDynamic(object.mPose);
			rigidDynamic->attachShape(*shape);

			rigidDynamic->setLinearVelocity(object.mLinearVelocity, false);
			rigidDynamic->setAngularVelocity(object.mAngularVelocity, false);

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
