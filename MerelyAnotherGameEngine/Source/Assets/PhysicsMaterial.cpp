#include "Assets/PhysicsMaterial.h"

#include <PxPhysicsAPI.h>

PhysicsMaterial::~PhysicsMaterial()
{
	if (mMaterial)
		PX_RELEASE(mMaterial);
}
