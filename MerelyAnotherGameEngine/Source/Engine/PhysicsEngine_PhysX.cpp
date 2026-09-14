#include "Engine/PhysicsEngine_PhysX.h"

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

physx::PxScene* PhysicsEngine_PhysX::CreateScene() const
{
	physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, 0.0f, -9.81f);
	sceneDesc.cpuDispatcher = mDispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	return mPhysics->createScene(sceneDesc);
}

physx::PxMaterial* PhysicsEngine_PhysX::CreateMaterial(MaterialProperties inProperties) const
{
	return mPhysics->createMaterial(
		inProperties.StaticFriction,
		inProperties.DynamicFriction,
		inProperties.Restitution);
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
