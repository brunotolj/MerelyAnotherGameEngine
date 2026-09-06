#include "Assets/PhysicsMaterialFactory.h"
#include "Engine/Engine.h"

AssetHandle<PhysicsMaterial> Factory<PhysicsMaterial>::Create(f32 mStaticFriction, f32 mDynamicFriction, f32 mRestitution)
{
	PhysicsMaterial* result = new PhysicsMaterial();

	result->mMaterial = gEngine->mPhysicsEngine.CreateMaterial({ mStaticFriction, mDynamicFriction, mRestitution });

	return gEngine->mAssetManager.Register(result);
}
