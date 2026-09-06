#include "Assets/PhysicsShape.h"

PhysicsShape::Geometry::Geometry(Geometry const& inOther)
{
	*this = inOther;
}

PhysicsShape::Geometry& PhysicsShape::Geometry::operator=(Geometry const& inOther)
{
	memcpy((void*)this, (void*)&inOther, sizeof(PhysicsShape::Geometry));
	if (Type == physx::PxGeometryType::eCUSTOM)
	{
		CustomGeometry.Geometry.callbacks = (physx::PxCustomGeometry::Callbacks*)&CustomGeometry.Callbacks;
	}

	return *this;
}

PhysicsShape::Geometry::Geometry(physx::PxCustomGeometryExt::CylinderCallbacks const& inCylinder)
{
	new (&CustomGeometry.Callbacks.CylinderCallbacks) physx::PxCustomGeometryExt::CylinderCallbacks(inCylinder);
	CustomGeometry.Geometry = physx::PxCustomGeometry(CustomGeometry.Callbacks.CylinderCallbacks);
}

PhysicsShape::Geometry::Geometry(physx::PxCustomGeometryExt::ConeCallbacks const& inCone)
{
	new (&CustomGeometry.Callbacks.ConeCallbacks) physx::PxCustomGeometryExt::ConeCallbacks(inCone);
	CustomGeometry.Geometry = physx::PxCustomGeometry(CustomGeometry.Callbacks.ConeCallbacks);
}
