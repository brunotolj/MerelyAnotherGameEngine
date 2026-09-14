#pragma once

#include "Assets/StaticMesh.h"
#include "Assets/Texture.h"
#include "Framework/GameWorld.h"
#include "Framework/TransformTree.h"
#include "Physics/PhysicsCommon.h"

struct GameplaySystemSetup
{
	f32 HalfSpan = 0.0f;
	f32 MaxSpeed = 0.0f;
	f32 Acceleration = 0.0f;
	f32 Deceleration = 0.0f;

	f32 BallSpawnInterval = 0.0f;
	PhysicsRigidBodyParams BallRigidBodyParams;
	AssetHandle<StaticMesh> BallMesh;
	AssetHandle<Texture> BallTexture;
};

class GameplaySystem : public GameSystemWithPrerequisites<TransformTree>
{
public:
	GameplaySystem(GameWorld& inWorld, GameplaySystemSetup const& inSetup);

	virtual void Update(f32 inDeltaTime) override;

	void SetupPlayer(u32 inPlayerIndex, TransformTreeEntryId inTransformId, i32 inInputCodeNegative, i32 inInputCodePositive);

	void AddBallSpawner(TransformTreeEntryId inTransformId, glm::vec3 inVelocity, f32 inVelocityVariance);

private:
	void SpawnBall();

	GameplaySystemSetup mSetup;

	f32 mBallSpawnTime = 0.0f;

	struct PlayerData
	{
		mage::Transform OriginalTransform;
		TransformTreeEntryId TransformId = mage::InvalidIndex;

		i32 InputCodeNegative = 0;
		i32 InputCodePositive = 0;

		f32 Position = 0.0f;
		f32 Speed = 0.0f;
	};

	struct BallSpawnerData
	{
		TransformTreeEntryId TransformId = mage::InvalidIndex;
		glm::vec3 Velocity = glm::vec3(0.0f);
		f32 VelocityVariance = 0.0f;
	};

	PlayerData mPlayerData[4];

	mage::Array<BallSpawnerData> mBallSpawners;
};
