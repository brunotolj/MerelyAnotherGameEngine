#include "Engine/Engine.h"
#include "Game/GameplaySystem.h"

GameplaySystem::GameplaySystem(GameWorld& inWorld, GameplaySystemSetup const& inSetup)
	: GameSystemWithPrerequisites(inWorld), mSetup(inSetup)
{
}

void GameplaySystem::Update(f32 inDeltaTime)
{
	if (mSetup.BallSpawnInterval > 0.0f)
	{
		mBallSpawnTime += inDeltaTime;
		while (mBallSpawnTime > mSetup.BallSpawnInterval)
		{
			mBallSpawnTime -= mSetup.BallSpawnInterval;
			SpawnBall();
		}
	}

	for (u32 i = 0; i < 4; ++i)
	{
		PlayerData& playerData = mPlayerData[i];

		f32 input = 0.0f;
		if (gEngine->mInputHandler.IsKeyPressed(playerData.InputCodeNegative)) input -= 1.0f;
		if (gEngine->mInputHandler.IsKeyPressed(playerData.InputCodePositive)) input += 1.0f;

		f32 remainingTime = inDeltaTime;
		f32 movement = 0.0f;

		if (remainingTime > 0.0f && playerData.Speed != 0.0f && input * playerData.Speed <= 0.0f)
		{
			f32 decelTime = std::fabsf(playerData.Speed) / mSetup.Deceleration;
			if (decelTime > remainingTime)
			{
				f32 deltaSpeed = playerData.Speed / std::fabsf(playerData.Speed) * mSetup.Deceleration * remainingTime;
				movement += (playerData.Speed - 0.5f * deltaSpeed) * remainingTime;
				playerData.Speed -= deltaSpeed;
				remainingTime = 0.0f;
			}
			else
			{
				movement += 0.5f * playerData.Speed * decelTime;
				playerData.Speed = 0.0f;
				remainingTime -= decelTime;
			}
		}

		if (remainingTime > 0.0f && ((playerData.Speed == 0.0f && input != 0.0f) || input * playerData.Speed > 0.0f))
		{
			f32 accelTime = (mSetup.MaxSpeed - input * playerData.Speed) / mSetup.Acceleration;
			if (accelTime > remainingTime)
			{
				f32 deltaSpeed = input * mSetup.Acceleration * remainingTime;
				movement += (playerData.Speed + 0.5f * deltaSpeed) * remainingTime;
				playerData.Speed += deltaSpeed;
				remainingTime = 0.0f;
			}
			else
			{
				movement += 0.5f * (playerData.Speed + input * mSetup.MaxSpeed) * accelTime;
				playerData.Speed = input * mSetup.MaxSpeed;
				remainingTime -= accelTime;
			}
		}

		movement += playerData.Speed * remainingTime;

		playerData.Position += movement;
		if (playerData.Position > mSetup.HalfSpan)
		{
			playerData.Position = mSetup.HalfSpan;
			playerData.Speed = 0.0f;
		}
		else if (playerData.Position < -mSetup.HalfSpan)
		{
			playerData.Position = -mSetup.HalfSpan;
			playerData.Speed = 0.0f;
		}

		mage::Transform transform = playerData.OriginalTransform;
		transform.Position += playerData.Position * playerData.OriginalTransform.Rotation.Rotate({ 0.0f, -1.0f, 0.0f });
		Get<TransformTree>().SetGlobalTransform(playerData.TransformId, transform);
	}
}

void GameplaySystem::SetupPlayer(u32 inPlayerIndex, TransformTreeEntryId inTransformId, i32 inInputCodeNegative, i32 inInputCodePositive)
{
	mage_check(inPlayerIndex < 4);

	PlayerData& playerData = mPlayerData[inPlayerIndex];
	playerData.OriginalTransform = Get<TransformTree>().GetGlobalTransform(inTransformId);
	playerData.TransformId = inTransformId;
	playerData.InputCodeNegative = inInputCodeNegative;
	playerData.InputCodePositive = inInputCodePositive;
}

void GameplaySystem::AddBallSpawner(TransformTreeEntryId inTransformId, glm::vec3 inVelocity, f32 inVelocityVariance)
{
	mBallSpawners.AddConstruct(inTransformId, inVelocity, inVelocityVariance);
}

void GameplaySystem::SpawnBall()
{
	if (mBallSpawners.GetSize() == 0)
		return;

	u32 index = rand() % mBallSpawners.GetSize();

	BallSpawnerData const& spawner = mBallSpawners[index];

	glm::vec3 velocity = Get<TransformTree>().GetGlobalTransform(spawner.TransformId).Rotation.Rotate(spawner.Velocity);
	velocity.x += (0.01f * (rand() % 100) - 0.5f) * spawner.VelocityVariance;
	velocity.y += (0.01f * (rand() % 100) - 0.5f) * spawner.VelocityVariance;
	velocity.z += (0.01f * (rand() % 100) - 0.5f) * spawner.VelocityVariance;

	mage::Transform const& transform = Get<TransformTree>().GetGlobalTransform(spawner.TransformId);
	physx::PxVec3 pxVelocity = reinterpret_cast<physx::PxVec3 const&>(velocity);

	TransformEntity* transformEntity = mWorld.CreateEntity<TransformEntity>(nullptr, transform);
	RigidBodyEntity* rigidBodyEntity = mWorld.CreateEntity<RigidBodyEntity>(*transformEntity, mSetup.BallRigidBodyParams, pxVelocity);
	StaticMeshEntity* staticMeshEntity = mWorld.CreateEntity<StaticMeshEntity>(*transformEntity, mSetup.BallMesh, mSetup.BallTexture);
}
