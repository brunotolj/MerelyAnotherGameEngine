#include "Assets/PhysicsShapeFactory.h"
#include "Engine/Engine.h"

AssetFactoryFunction PhysicsShapeFactoryFunction("PhysicsShape", [](mage::StringView inName, PropertyContainer const& inProperties) { return Factory<PhysicsShape>::Create(inName, inProperties); });

AssetHandle<PhysicsShape> Factory<PhysicsShape>::Create(mage::StringView inName, PropertyContainer const& inProperties)
{
	auto getProperty = [&inProperties](mage::StringView inPropertyName) { return inProperties.contains(inPropertyName) ? inProperties.at(inPropertyName) : ""; };

	mage::StringView type = getProperty("type");

	if (type == "Box")
	{
		f32 halfX = mage::ParseNumber<f32>(getProperty("halfX"));
		f32 halfY = mage::ParseNumber<f32>(getProperty("halfY"));
		f32 halfZ = mage::ParseNumber<f32>(getProperty("halfZ"));

		return MakeBox(inName, { halfX, halfY, halfZ });
	}
	else if (type == "Sphere")
	{
		f32 radius = mage::ParseNumber<f32>(getProperty("radius"));

		return MakeSphere(inName, radius);
	}
	else if (type == "Cylinder")
	{
		f32 radius = mage::ParseNumber<f32>(getProperty("radius"));
		f32 halfHeight = mage::ParseNumber<f32>(getProperty("halfHeight"));

		return MakeCylinder(inName, radius, halfHeight);
	}
	else if (type == "Capsule")
	{
		f32 radius = mage::ParseNumber<f32>(getProperty("radius"));
		f32 halfHeight = mage::ParseNumber<f32>(getProperty("halfHeight"));

		return MakeCapsule(inName, radius, halfHeight);
	}
	else if (type == "Cone")
	{
		f32 radius = mage::ParseNumber<f32>(getProperty("radius"));
		f32 height = mage::ParseNumber<f32>(getProperty("height"));

		return MakeCone(inName, radius, height);
	}

	return nullptr;
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::MakeBox(mage::StringView inName, glm::vec3 inHalfExtent)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxBoxGeometry(inHalfExtent.x, inHalfExtent.y, inHalfExtent.z);

	return gEngine->mAssetManager.Register(result, inName);
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::MakeSphere(mage::StringView inName, f32 inRadius)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxSphereGeometry(inRadius);

	return gEngine->mAssetManager.Register(result, inName);
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::MakeCapsule(mage::StringView inName, f32 inRadius, f32 inHalfHeight)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxCapsuleGeometry(inRadius, inHalfHeight);

	return gEngine->mAssetManager.Register(result, inName);
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::MakeCylinder(mage::StringView inName, f32 inRadius, f32 inHalfHeight)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxCustomGeometryExt::CylinderCallbacks(2.0f * inHalfHeight, inRadius);

	return gEngine->mAssetManager.Register(result, inName);
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::MakeCone(mage::StringView inName, f32 inRadius, f32 inHeight)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxCustomGeometryExt::ConeCallbacks(inHeight, inRadius);

	return gEngine->mAssetManager.Register(result, inName);
}
