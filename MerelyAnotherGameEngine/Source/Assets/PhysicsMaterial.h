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
	static AssetHandle<PhysicsMaterial> Create(mage::StringView inName, PropertyContainer const& inProperties);

private:
	Factory() {}
};
