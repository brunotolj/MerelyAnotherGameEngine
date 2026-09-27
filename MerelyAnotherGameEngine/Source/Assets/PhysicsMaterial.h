#pragma once

#include "Assets/Asset.h"

namespace physx
{
	class PxMaterial;
}

class PhysicsMaterial : public Asset
{
	friend class Factory<PhysicsMaterial>;

public:
	physx::PxMaterial const& GetRaw() const { return *mMaterial; }

private:
	PhysicsMaterial() {}
	virtual ~PhysicsMaterial();

	physx::PxMaterial* mMaterial = nullptr;
};

template<>
class Factory<PhysicsMaterial>
{
public:
	static AssetHandle<PhysicsMaterial> FromFile(mage::StringView inPath) { return nullptr; }

	struct Impl
	{
		AssetHandle<PhysicsMaterial> Create(mage::StringView inName);

		f32 StaticFriction = 1.0f;
		f32 DynamicFriction = 1.0f;
		f32 Restitution = 1.0f;
	};

private:
	Factory() {}
};

template <>
struct Property<Factory<PhysicsMaterial>::Impl> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(Factory<PhysicsMaterial>::Impl, StaticFriction);
		inOutContainer.AddProperty(Factory<PhysicsMaterial>::Impl, DynamicFriction);
		inOutContainer.AddProperty(Factory<PhysicsMaterial>::Impl, Restitution);
	}
};
