#pragma once

#include "Assets/PhysicsMaterial.h"
#include "Assets/PhysicsShape.h"
#include "Assets/StaticMesh.h"
#include "Assets/Texture.h"
#include "Framework/GameWorld.h"
#include "Framework/TransformTree.h"

class GameplaySystem : public GameSystemWithPrerequisites<TransformTree>
{
public:
	GameplaySystem(GameWorld& inWorld);

	virtual void Update(f32 inDeltaTime) override;

	f32 HalfSpan = 0.0f;
	f32 MaxSpeed = 0.0f;
	f32 Acceleration = 0.0f;
	f32 Deceleration = 0.0f;

	f32 BallSpawnInterval = 0.0f;
	AssetHandle<PhysicsShape> BallPhysicsShape = nullptr;
	AssetHandle<PhysicsMaterial> BallPhysicsMaterial = nullptr;
	AssetHandle<StaticMesh> BallMesh = nullptr;
	AssetHandle<Texture> BallTexture = nullptr;

private:
	void SpawnBall();

	f32 mBallSpawnTime = 0.0f;
};

template <>
struct Property<GameplaySystem> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(GameplaySystem, HalfSpan);
		inOutContainer.AddProperty(GameplaySystem, MaxSpeed);
		inOutContainer.AddProperty(GameplaySystem, Acceleration);
		inOutContainer.AddProperty(GameplaySystem, Deceleration);
		inOutContainer.AddProperty(GameplaySystem, BallSpawnInterval);
		inOutContainer.AddProperty(GameplaySystem, BallPhysicsShape);
		inOutContainer.AddProperty(GameplaySystem, BallPhysicsMaterial);
		inOutContainer.AddProperty(GameplaySystem, BallMesh);
		inOutContainer.AddProperty(GameplaySystem, BallTexture);
	}
};
