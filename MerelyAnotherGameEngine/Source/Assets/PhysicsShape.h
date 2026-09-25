#pragma once

#include "Assets/Asset.h"
#include "Property.h"

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

	struct Box
	{
		AssetHandle<PhysicsShape> Create(mage::StringView inName);

		glm::vec3 HalfExtent{ 1.0f, 1.0f, 1.0f };
	};

	struct Sphere
	{
		AssetHandle<PhysicsShape> Create(mage::StringView inName);

		f32 Radius = 1.0f;
	};

	struct Cylinder
	{
		AssetHandle<PhysicsShape> Create(mage::StringView inName);

		f32 Radius = 1.0f;
		f32 HalfHeight = 1.0f;
	};

	struct Capsule
	{
		AssetHandle<PhysicsShape> Create(mage::StringView inName);

		f32 Radius = 1.0f;
		f32 HalfHeight = 1.0f;
	};

	struct Cone
	{
		AssetHandle<PhysicsShape> Create(mage::StringView inName);

		f32 Radius = 1.0f;
		f32 Height = 1.0f;
	};

private:
	Factory() {}
};

template <>
struct Property<Factory<PhysicsShape>::Box> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(Factory<PhysicsShape>::Box, HalfExtent);
	}
};

template <>
struct Property<Factory<PhysicsShape>::Sphere> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(Factory<PhysicsShape>::Sphere, Radius);
	}
};

template <>
struct Property<Factory<PhysicsShape>::Cylinder> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(Factory<PhysicsShape>::Cylinder, Radius);
		inOutContainer.AddProperty(Factory<PhysicsShape>::Cylinder, HalfHeight);
	}
};

template <>
struct Property<Factory<PhysicsShape>::Capsule> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(Factory<PhysicsShape>::Capsule, Radius);
		inOutContainer.AddProperty(Factory<PhysicsShape>::Capsule, HalfHeight);
	}
};

template <>
struct Property<Factory<PhysicsShape>::Cone> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(Factory<PhysicsShape>::Cone, Radius);
		inOutContainer.AddProperty(Factory<PhysicsShape>::Cone, Height);
	}
};
