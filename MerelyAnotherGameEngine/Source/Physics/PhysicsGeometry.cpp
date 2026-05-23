#pragma once

#include "Physics/PhysicsGeometry.h"

PhysicsGeometry::PhysicsGeometry(PhysicsGeometry const& inOther)
{
	*this = inOther;
}

PhysicsGeometry& PhysicsGeometry::operator=(PhysicsGeometry const& inOther)
{
	memcpy((void*)this, (void*)&inOther, sizeof(PhysicsGeometry));
	if (Type == physx::PxGeometryType::eCUSTOM)
	{
		CustomGeometry.Geometry.callbacks = (physx::PxCustomGeometry::Callbacks*)&CustomGeometry.Callbacks;
	}

	return *this;
}

PhysicsGeometry::PhysicsGeometry(physx::PxCustomGeometryExt::CylinderCallbacks& inCylinder)
{
	new (&CustomGeometry.Callbacks.CylinderCallbacks) physx::PxCustomGeometryExt::CylinderCallbacks(inCylinder);
	CustomGeometry.Geometry = physx::PxCustomGeometry(inCylinder);
}

PhysicsGeometry::PhysicsGeometry(physx::PxCustomGeometryExt::ConeCallbacks& inCone)
{
	new (&CustomGeometry.Callbacks.ConeCallbacks) physx::PxCustomGeometryExt::ConeCallbacks(inCone);
	CustomGeometry.Geometry = physx::PxCustomGeometry(inCone);
}
