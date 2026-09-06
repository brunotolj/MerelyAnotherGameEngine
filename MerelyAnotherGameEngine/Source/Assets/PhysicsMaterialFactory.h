#pragma once

#include "Assets/PhysicsMaterial.h"

template<>
class Factory<PhysicsMaterial>
{
public:
	static AssetHandle<PhysicsMaterial> Create(f32 mStaticFriction, f32 mDynamicFriction, f32 mRestitution);

private:
	Factory() {}
};
