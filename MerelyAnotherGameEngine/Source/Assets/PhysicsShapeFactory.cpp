#include "Assets/PhysicsShape.h"
#include "Engine/Engine.h"

REGISTER_ASSET_FACTORY_FUNCTION_WITH_SUBTYPE(PhysicsShape, Box);
REGISTER_ASSET_FACTORY_FUNCTION_WITH_SUBTYPE(PhysicsShape, Sphere);
REGISTER_ASSET_FACTORY_FUNCTION_WITH_SUBTYPE(PhysicsShape, Cylinder);
REGISTER_ASSET_FACTORY_FUNCTION_WITH_SUBTYPE(PhysicsShape, Capsule);
REGISTER_ASSET_FACTORY_FUNCTION_WITH_SUBTYPE(PhysicsShape, Cone);

AssetHandle<PhysicsShape> Factory<PhysicsShape>::Box::Create(mage::StringView inName)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxBoxGeometry(HalfExtent.x, HalfExtent.y, HalfExtent.z);

	return gEngine->mAssetManager.Register(result, inName);
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::Sphere::Create(mage::StringView inName)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxSphereGeometry(Radius);

	return gEngine->mAssetManager.Register(result, inName);
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::Capsule::Create(mage::StringView inName)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxCapsuleGeometry(Radius, HalfHeight);

	return gEngine->mAssetManager.Register(result, inName);
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::Cylinder::Create(mage::StringView inName)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxCustomGeometryExt::CylinderCallbacks(2.0f * HalfHeight, Radius);

	return gEngine->mAssetManager.Register(result, inName);
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::Cone::Create(mage::StringView inName)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxCustomGeometryExt::ConeCallbacks(Height, Radius);

	return gEngine->mAssetManager.Register(result, inName);
}
