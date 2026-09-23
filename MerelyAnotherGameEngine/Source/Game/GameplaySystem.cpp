#include "Engine/Engine.h"
#include "Game/GameplayEntities.h"
#include "Game/GameplaySystem.h"

WorldComponentFactoryFunction GameplaySystemFactoryFunction("GameplaySystem", [](GameWorld& inWorld, PropertyContainer const& inProperties)
{
	auto getProperty = [&inProperties](mage::StringView inPropertyName) { return inProperties.contains(inPropertyName) ? inProperties.at(inPropertyName) : ""; };

	GameplaySystemSetup setup;
	setup.HalfSpan = mage::ParseNumber<f32>(getProperty("halfSpan"));
	setup.MaxSpeed = mage::ParseNumber<f32>(getProperty("maxSpeed"));
	setup.Acceleration = mage::ParseNumber<f32>(getProperty("acceleration"));
	setup.Deceleration = mage::ParseNumber<f32>(getProperty("deceleration"));
	setup.BallSpawnInterval = mage::ParseNumber<f32>(getProperty("ballSpawnInterval"));
	setup.BallPhysicsShape = gEngine->mAssetManager.GetHandle<PhysicsShape>(getProperty("ballPhysicsShape"));
	setup.BallPhysicsMaterial = gEngine->mAssetManager.GetHandle<PhysicsMaterial>(getProperty("ballPhysicsMaterial"));
	setup.BallMesh = gEngine->mAssetManager.GetHandle<StaticMesh>(getProperty("ballMesh"));
	setup.BallTexture = gEngine->mAssetManager.GetHandle<Texture>(getProperty("ballTexture"));

	inWorld.CreateComponent<GameplaySystem>(setup);
});

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

	for (PlayerEntity* playerEntity : mWorld.GetEntities<PlayerEntity>())
	{
		f32 input = 0.0f;
		if (gEngine->mInputHandler.IsKeyPressed(playerEntity->mInputCodeNegative)) input -= 1.0f;
		if (gEngine->mInputHandler.IsKeyPressed(playerEntity->mInputCodePositive)) input += 1.0f;

		f32 remainingTime = inDeltaTime;
		f32 movement = 0.0f;

		if (remainingTime > 0.0f && playerEntity->mSpeed != 0.0f && input * playerEntity->mSpeed <= 0.0f)
		{
			f32 decelTime = std::fabsf(playerEntity->mSpeed) / mSetup.Deceleration;
			if (decelTime > remainingTime)
			{
				f32 deltaSpeed = playerEntity->mSpeed / std::fabsf(playerEntity->mSpeed) * mSetup.Deceleration * remainingTime;
				movement += (playerEntity->mSpeed - 0.5f * deltaSpeed) * remainingTime;
				playerEntity->mSpeed -= deltaSpeed;
				remainingTime = 0.0f;
			}
			else
			{
				movement += 0.5f * playerEntity->mSpeed * decelTime;
				playerEntity->mSpeed = 0.0f;
				remainingTime -= decelTime;
			}
		}

		if (remainingTime > 0.0f && ((playerEntity->mSpeed == 0.0f && input != 0.0f) || input * playerEntity->mSpeed > 0.0f))
		{
			f32 accelTime = (mSetup.MaxSpeed - input * playerEntity->mSpeed) / mSetup.Acceleration;
			if (accelTime > remainingTime)
			{
				f32 deltaSpeed = input * mSetup.Acceleration * remainingTime;
				movement += (playerEntity->mSpeed + 0.5f * deltaSpeed) * remainingTime;
				playerEntity->mSpeed += deltaSpeed;
				remainingTime = 0.0f;
			}
			else
			{
				movement += 0.5f * (playerEntity->mSpeed + input * mSetup.MaxSpeed) * accelTime;
				playerEntity->mSpeed = input * mSetup.MaxSpeed;
				remainingTime -= accelTime;
			}
		}

		movement += playerEntity->mSpeed * remainingTime;

		playerEntity->mPosition += movement;
		if (playerEntity->mPosition > mSetup.HalfSpan)
		{
			playerEntity->mPosition = mSetup.HalfSpan;
			playerEntity->mSpeed = 0.0f;
		}
		else if (playerEntity->mPosition < -mSetup.HalfSpan)
		{
			playerEntity->mPosition = -mSetup.HalfSpan;
			playerEntity->mSpeed = 0.0f;
		}

		mage::Transform transform = playerEntity->mOriginalTransform;
		transform.Position += playerEntity->mPosition * playerEntity->mOriginalTransform.Rotation.Rotate({ 0.0f, -1.0f, 0.0f });
		Get<TransformTree>().SetGlobalTransform(playerEntity->mTransformId, transform);
	}
}

void GameplaySystem::SpawnBall()
{
	mage::Array<BallSpawnerEntity*> const& ballSpawners = mWorld.GetEntities<BallSpawnerEntity>();
	if (ballSpawners.GetSize() == 0)
		return;

	u32 index = rand() % ballSpawners.GetSize();

	BallSpawnerEntity* spawner = ballSpawners[index];

	glm::vec3 velocity = Get<TransformTree>().GetGlobalTransform(spawner->GetParentEntity().mTransformId).Rotation.Rotate(spawner->mSpawnVelocity);
	velocity.x += (0.01f * (rand() % 100) - 0.5f) * spawner->mSpawnVelocityVariance.x;
	velocity.y += (0.01f * (rand() % 100) - 0.5f) * spawner->mSpawnVelocityVariance.y;
	velocity.z += (0.01f * (rand() % 100) - 0.5f) * spawner->mSpawnVelocityVariance.z;

	mage::Transform const& transform = Get<TransformTree>().GetGlobalTransform(spawner->GetParentEntity().mTransformId);

	TransformEntity* transformEntity = mWorld.CreateEntity<TransformEntity>(nullptr, transform);
	mWorld.CreateEntity<DynamicRigidBodyEntity>(*transformEntity, mSetup.BallPhysicsShape, mSetup.BallPhysicsMaterial, false, velocity);
	mWorld.CreateEntity<StaticMeshEntity>(*transformEntity, mSetup.BallMesh, mSetup.BallTexture);
}
