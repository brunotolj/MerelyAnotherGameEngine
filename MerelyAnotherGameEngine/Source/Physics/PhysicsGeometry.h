#pragma once

#include <Geometry/PxCustomGeometry.h>
#include <PxPhysicsAPI.h>

union PhysicsGeometry
{
public:
	PhysicsGeometry() : Type(physx::PxGeometryType::eINVALID) {}
	~PhysicsGeometry() {}
	
	PhysicsGeometry(PhysicsGeometry const& inOther);
	PhysicsGeometry& operator=(PhysicsGeometry const& inOther);

	PhysicsGeometry(physx::PxSphereGeometry const& inSphere) : SphereGeometry(inSphere) {}
	PhysicsGeometry(physx::PxCapsuleGeometry const& inCapsule) : CapsuleGeometry(inCapsule) {}
	PhysicsGeometry(physx::PxBoxGeometry const& inBox) : BoxGeometry(inBox) {}

	PhysicsGeometry(physx::PxCustomGeometryExt::CylinderCallbacks& inCylinder);
	PhysicsGeometry(physx::PxCustomGeometryExt::ConeCallbacks& inCone);
	
	physx::PxGeometryType::Enum Type;
	physx::PxGeometry BaseGeometry;
	physx::PxSphereGeometry SphereGeometry;
	physx::PxCapsuleGeometry CapsuleGeometry;
	physx::PxBoxGeometry BoxGeometry;

	struct CustomGeometry
	{
		~CustomGeometry() {}

		physx::PxCustomGeometry Geometry;

		union CustomCallback
		{
			~CustomCallback() {}

			physx::PxCustomGeometryExt::CylinderCallbacks CylinderCallbacks;
			physx::PxCustomGeometryExt::ConeCallbacks ConeCallbacks;

		} Callbacks;

	} CustomGeometry;
};
