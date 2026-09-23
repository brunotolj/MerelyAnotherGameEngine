#pragma once

#include "Assets/PhysicsShape.h"

template<>
class Factory<PhysicsShape>
{
public:
	static AssetHandle<PhysicsShape> Create(mage::StringView inName, PropertyContainer const& inProperties);

	static AssetHandle<PhysicsShape> MakeBox(mage::StringView inName, glm::vec3 inHalfExtent);
	static AssetHandle<PhysicsShape> MakeSphere(mage::StringView inName, f32 inRadius);
	static AssetHandle<PhysicsShape> MakeCapsule(mage::StringView inName, f32 inRadius, f32 inHalfHeight);
	static AssetHandle<PhysicsShape> MakeCylinder(mage::StringView inName, f32 inRadius, f32 inHalfHeight);
	static AssetHandle<PhysicsShape> MakeCone(mage::StringView inName, f32 inRadius, f32 inHeight);

private:
	Factory() {}
};
