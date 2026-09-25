#include "Assets/PhysicsMaterial.h"
#include "Engine/Engine.h"

REGISTER_ASSET_FACTORY_FUNCTION(PhysicsMaterial);

AssetHandle<PhysicsMaterial> Factory<PhysicsMaterial>::Impl::Create(mage::StringView inName)
{
	if (StaticFriction < 0.0f) return nullptr;
	if (DynamicFriction < 0.0f) return nullptr;
	if (Restitution < 0.0f || Restitution > 1.0f) return nullptr;

	PhysicsMaterial* result = new PhysicsMaterial();

	result->mMaterial = gEngine->mPhysicsEngine.CreateMaterial(StaticFriction, DynamicFriction, Restitution);

	return gEngine->mAssetManager.Register(result, inName);
}
