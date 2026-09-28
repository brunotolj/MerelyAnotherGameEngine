#include "Engine/PhysicsEngine_PhysX.h"

void PhysicsCallbacks::onTrigger(physx::PxTriggerPair* inPairs, physx::PxU32 inCount)
{
	for (physx::PxU32 i = 0; i < inCount; i++)
	{
		physx::PxTriggerPair& pair = inPairs[i];

		physx::PxActor* triggerActor = pair.triggerActor;
		physx::PxActor* otherActor = pair.otherActor;

		if (pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			mTriggerOverlapEvents.AddConstruct(triggerActor, otherActor, true);
		}
		else if (pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
		{
			mTriggerOverlapEvents.AddConstruct(triggerActor, otherActor, false);
		}
	}
}

PhysicsEngine_PhysX::PhysicsEngine_PhysX()
{
	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);
	mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale());
	mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
}

PhysicsEngine_PhysX::~PhysicsEngine_PhysX()
{
	PX_RELEASE(mDispatcher);
	PX_RELEASE(mPhysics);
	PX_RELEASE(mFoundation);
}

physx::PxScene* PhysicsEngine_PhysX::CreateScene(PhysicsCallbacks& inCallbacks) const
{
	physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, 0.0f, -9.81f);
	sceneDesc.cpuDispatcher = mDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	sceneDesc.simulationEventCallback = &inCallbacks;
	return mPhysics->createScene(sceneDesc);
}

physx::PxMaterial* PhysicsEngine_PhysX::CreateMaterial(f32 inStaticFriction, f32 inDynamicFriction, f32 inRestitution) const
{
	return mPhysics->createMaterial(inStaticFriction, inDynamicFriction, inRestitution);
}

physx::PxShape* PhysicsEngine_PhysX::CreateShape(AssetHandle<PhysicsShape> inShape, AssetHandle<PhysicsMaterial> inMaterial) const
{
	PhysicsShape const* shapeAsset = inShape.GetAsset();
	mage_check(shapeAsset);

	PhysicsMaterial const* materialAsset = inMaterial.GetAsset();
	mage_check(materialAsset);

	return mPhysics->createShape(shapeAsset->GetRaw(), materialAsset->GetRaw(), true);
}

physx::PxRigidStatic* PhysicsEngine_PhysX::CreateStaticActor(physx::PxTransform const& inPose) const
{
	return mPhysics->createRigidStatic(inPose);
}

physx::PxRigidDynamic* PhysicsEngine_PhysX::CreateDynamicActor(physx::PxTransform const& inPose) const
{
	return mPhysics->createRigidDynamic(inPose);
}
