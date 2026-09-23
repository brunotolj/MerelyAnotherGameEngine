#pragma once

#include "Assets/PhysicsMaterial.h"

template<>
class Factory<PhysicsMaterial>
{
public:
	static AssetHandle<PhysicsMaterial> Create(mage::StringView inName, PropertyContainer const& inProperties);

private:
	Factory() {}
};
