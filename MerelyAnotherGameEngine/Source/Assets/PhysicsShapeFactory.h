#pragma once

#include "Assets/PhysicsShape.h"

template<>
class Factory<PhysicsShape>
{
public:
	static AssetHandle<PhysicsShape> MakeBox(glm::vec3 inHalfExtent);
	static AssetHandle<PhysicsShape> MakeSphere(f32 inRadius);
	static AssetHandle<PhysicsShape> MakeCapsule(f32 inRadius, f32 inLength);
	static AssetHandle<PhysicsShape> MakeCylinder(f32 inRadius, f32 inLength);
	static AssetHandle<PhysicsShape> MakeCone(f32 inRadius, f32 inHeight);

private:
	Factory() {}
};
