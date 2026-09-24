#pragma once

#include "Assets/Asset.h"

#include <PxPhysicsAPI.h>

class PhysicsShape : public Asset
{
	friend class Factory<PhysicsShape>;

public:
	physx::PxGeometry const& GetRaw() const { return mGeometry.BaseGeometry; }

private:
	PhysicsShape() {}

	union Geometry
	{
	public:
		Geometry() : Type(physx::PxGeometryType::eINVALID) {}
		~Geometry() {}

		Geometry(Geometry const& inOther);
		Geometry& operator=(Geometry const& inOther);

		Geometry(physx::PxSphereGeometry const& inSphere) : SphereGeometry(inSphere) {}
		Geometry(physx::PxCapsuleGeometry const& inCapsule) : CapsuleGeometry(inCapsule) {}
		Geometry(physx::PxBoxGeometry const& inBox) : BoxGeometry(inBox) {}

		Geometry(physx::PxCustomGeometryExt::CylinderCallbacks const& inCylinder);
		Geometry(physx::PxCustomGeometryExt::ConeCallbacks const& inCone);

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
	} mGeometry;
};

template<>
class Factory<PhysicsShape>
{
public:
	static AssetHandle<PhysicsShape> FromFile(mage::StringView inPath) { return nullptr; }
	static AssetHandle<PhysicsShape> Create(mage::StringView inName, PropertyContainer const& inProperties);

	static AssetHandle<PhysicsShape> MakeBox(mage::StringView inName, glm::vec3 inHalfExtent);
	static AssetHandle<PhysicsShape> MakeSphere(mage::StringView inName, f32 inRadius);
	static AssetHandle<PhysicsShape> MakeCapsule(mage::StringView inName, f32 inRadius, f32 inHalfHeight);
	static AssetHandle<PhysicsShape> MakeCylinder(mage::StringView inName, f32 inRadius, f32 inHalfHeight);
	static AssetHandle<PhysicsShape> MakeCone(mage::StringView inName, f32 inRadius, f32 inHeight);

private:
	Factory() {}
};
