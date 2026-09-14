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
