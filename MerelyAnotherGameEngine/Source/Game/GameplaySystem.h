#pragma once

#include "Assets/PhysicsMaterial.h"
#include "Assets/PhysicsShape.h"
#include "Assets/StaticMesh.h"
#include "Assets/Texture.h"
#include "Framework/GameWorld.h"
#include "Framework/TransformTree.h"

struct GameplaySystemSetup
{
	f32 HalfSpan = 0.0f;
	f32 MaxSpeed = 0.0f;
	f32 Acceleration = 0.0f;
	f32 Deceleration = 0.0f;

	f32 BallSpawnInterval = 0.0f;
	AssetHandle<PhysicsShape> BallPhysicsShape;
	AssetHandle<PhysicsMaterial> BallPhysicsMaterial;
	AssetHandle<StaticMesh> BallMesh;
	AssetHandle<Texture> BallTexture;
};

class GameplaySystem : public GameSystemWithPrerequisites<TransformTree>
{
public:
	GameplaySystem(GameWorld& inWorld, GameplaySystemSetup const& inSetup);

	virtual void Update(f32 inDeltaTime) override;

private:
	void SpawnBall();

	GameplaySystemSetup mSetup;

	f32 mBallSpawnTime = 0.0f;
};
