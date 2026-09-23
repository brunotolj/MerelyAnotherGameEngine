#include "Assets/PhysicsMaterialFactory.h"
#include "Engine/Engine.h"

AssetFactoryFunction PhysicsMaterialFactoryFunction("PhysicsMaterial", [](mage::StringView inName, PropertyContainer const& inProperties) { return Factory<PhysicsMaterial>::Create(inName, inProperties); });

AssetHandle<PhysicsMaterial> Factory<PhysicsMaterial>::Create(mage::StringView inName, PropertyContainer const& inProperties)
{
	auto getProperty = [&inProperties](mage::StringView inPropertyName) { return inProperties.contains(inPropertyName) ? inProperties.at(inPropertyName) : ""; };

	f32 staticFriction = mage::ParseNumber<f32>(getProperty("staticFriction"));
	if (staticFriction < 0.0f || staticFriction > 1.0f) return nullptr;

	f32 dynamicFriction = mage::ParseNumber<f32>(getProperty("dynamicFriction"));
	if (dynamicFriction < 0.0f || dynamicFriction > 1.0f) return nullptr;

	f32 restitution = mage::ParseNumber<f32>(getProperty("restitution"));
	if (restitution < 0.0f || restitution > 1.0f) return nullptr;

	PhysicsMaterial* result = new PhysicsMaterial();

	result->mMaterial = gEngine->mPhysicsEngine.CreateMaterial(staticFriction, dynamicFriction, restitution);

	return gEngine->mAssetManager.Register(result, inName);
}
