#pragma once

#include "Assets/PhysicsMaterial.h"
#include "Assets/PhysicsShape.h"
#include "Framework/GameWorld.h"
#include "Framework/TransformTree.h"

class RigidBodyObjectComponent;

class PhysicsSystem : public GameSystemWithPrerequisites<TransformTree>
{
public:
	PhysicsSystem(GameWorld& inWorld);
	virtual ~PhysicsSystem();

	virtual void GetEntityCallbackTypes(mage::Array<std::type_index>& outTypes) const override;

	virtual void OnEntityCreated(GameEntity* inEntity, GameEntitySetup const* inSetup, std::type_index inType) override;
	virtual void OnEntityDestroyed(GameEntity* inEntity, std::type_index inType) override;

	virtual void Update(f32 inDeltaTime) override;

	physx::PxRigidStatic* CreateStaticRigidBody(TransformTreeEntryId inTransformId, AssetHandle<PhysicsShape> inShape, AssetHandle<PhysicsMaterial> inMaterial);

	physx::PxRigidDynamic* CreateDynamicRigidBody(TransformTreeEntryId inTransformId, AssetHandle<PhysicsShape> inShape, AssetHandle<PhysicsMaterial> inMaterial,
		bool inIsKinematic, glm::vec3 inLinearVelocity, glm::vec3 inAngularVelocity);

	physx::PxRigidStatic* CreateStaticTriggerVolume(TransformTreeEntryId inTransformId, AssetHandle<PhysicsShape> inShape);

	void RemoveActor(physx::PxRigidActor* inActor);

private:
	physx::PxTransform ReadTransformFromTransformTree(TransformTreeEntryId inTransformId);
	void WriteTransformToTransformTree(TransformTreeEntryId inTransformId, physx::PxTransform const& inTransform);

	PhysicsCallbacks mCallbacks;

	physx::PxScene* mScene = nullptr;

	AssetHandle<PhysicsMaterial> mDummyMaterial = nullptr;
};
