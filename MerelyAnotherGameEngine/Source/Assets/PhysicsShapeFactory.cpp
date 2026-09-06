#include "Assets/PhysicsShapeFactory.h"
#include "Engine/Engine.h"

AssetHandle<PhysicsShape> Factory<PhysicsShape>::MakeBox(glm::vec3 inHalfExtent)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxBoxGeometry(inHalfExtent.x, inHalfExtent.y, inHalfExtent.z);

	return gEngine->mAssetManager.Register(result);
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::MakeSphere(f32 inRadius)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxSphereGeometry(inRadius);

	return gEngine->mAssetManager.Register(result);
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::MakeCapsule(f32 inRadius, f32 inLength)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxCapsuleGeometry(inRadius, inLength);

	return gEngine->mAssetManager.Register(result);
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::MakeCylinder(f32 inRadius, f32 inLength)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxCustomGeometryExt::CylinderCallbacks(inLength, inRadius);

	return gEngine->mAssetManager.Register(result);
}

AssetHandle<PhysicsShape> Factory<PhysicsShape>::MakeCone(f32 inRadius, f32 inHeight)
{
	PhysicsShape* result = new PhysicsShape();

	result->mGeometry = physx::PxCustomGeometryExt::ConeCallbacks(inHeight, inRadius);

	return gEngine->mAssetManager.Register(result);
}
