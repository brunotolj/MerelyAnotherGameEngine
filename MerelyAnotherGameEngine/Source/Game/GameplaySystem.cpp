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

	for (CapsuleMoverEntity* capsuleMoverEntity : mWorld.GetEntities<CapsuleMoverEntity>())
	{
		f32 input = 0.0f;
		if (gEngine->mInputHandler.IsKeyPressed(capsuleMoverEntity->mInputCodeNegative)) input -= 1.0f;
		if (gEngine->mInputHandler.IsKeyPressed(capsuleMoverEntity->mInputCodePositive)) input += 1.0f;

		f32 remainingTime = inDeltaTime;
		f32 movement = 0.0f;

		if (remainingTime > 0.0f && capsuleMoverEntity->mSpeed != 0.0f && input * capsuleMoverEntity->mSpeed <= 0.0f)
		{
			f32 decelTime = std::fabsf(capsuleMoverEntity->mSpeed) / mSetup.Deceleration;
			if (decelTime > remainingTime)
			{
				f32 deltaSpeed = capsuleMoverEntity->mSpeed / std::fabsf(capsuleMoverEntity->mSpeed) * mSetup.Deceleration * remainingTime;
				movement += (capsuleMoverEntity->mSpeed - 0.5f * deltaSpeed) * remainingTime;
				capsuleMoverEntity->mSpeed -= deltaSpeed;
				remainingTime = 0.0f;
			}
			else
			{
				movement += 0.5f * capsuleMoverEntity->mSpeed * decelTime;
				capsuleMoverEntity->mSpeed = 0.0f;
				remainingTime -= decelTime;
			}
		}

		if (remainingTime > 0.0f && ((capsuleMoverEntity->mSpeed == 0.0f && input != 0.0f) || input * capsuleMoverEntity->mSpeed > 0.0f))
		{
			f32 accelTime = (mSetup.MaxSpeed - input * capsuleMoverEntity->mSpeed) / mSetup.Acceleration;
			if (accelTime > remainingTime)
			{
				f32 deltaSpeed = input * mSetup.Acceleration * remainingTime;
				movement += (capsuleMoverEntity->mSpeed + 0.5f * deltaSpeed) * remainingTime;
				capsuleMoverEntity->mSpeed += deltaSpeed;
				remainingTime = 0.0f;
			}
			else
			{
				movement += 0.5f * (capsuleMoverEntity->mSpeed + input * mSetup.MaxSpeed) * accelTime;
				capsuleMoverEntity->mSpeed = input * mSetup.MaxSpeed;
				remainingTime -= accelTime;
			}
		}

		movement += capsuleMoverEntity->mSpeed * remainingTime;

		capsuleMoverEntity->mPosition += movement;
		if (capsuleMoverEntity->mPosition > mSetup.HalfSpan)
		{
			capsuleMoverEntity->mPosition = mSetup.HalfSpan;
			capsuleMoverEntity->mSpeed = 0.0f;
		}
		else if (capsuleMoverEntity->mPosition < -mSetup.HalfSpan)
		{
			capsuleMoverEntity->mPosition = -mSetup.HalfSpan;
			capsuleMoverEntity->mSpeed = 0.0f;
		}

		mage::Transform transform = capsuleMoverEntity->mOriginalTransform;
		transform.Position += capsuleMoverEntity->mPosition * capsuleMoverEntity->mOriginalTransform.Rotation.Rotate({ 0.0f, -1.0f, 0.0f });
		Get<TransformTree>().SetGlobalTransform(capsuleMoverEntity->GetParentEntity().mTransformId, transform);
	}
}

void GameplaySystem::SpawnBall()
{
	mage::Array<BallSpawnerEntity*> const& ballSpawners = mWorld.GetEntities<BallSpawnerEntity>();
	if (ballSpawners.GetSize() == 0)
		return;

	u32 index = rand() % ballSpawners.GetSize();
	BallSpawnerEntity* spawner = ballSpawners[index];

	glm::vec3 velocity = spawner->mSpawnVelocity;
	velocity.x += (0.01f * (rand() % 100) - 0.5f) * spawner->mSpawnVelocityVariance.x;
	velocity.y += (0.01f * (rand() % 100) - 0.5f) * spawner->mSpawnVelocityVariance.y;
	velocity.z += (0.01f * (rand() % 100) - 0.5f) * spawner->mSpawnVelocityVariance.z;

	mage::Transform const& transform = Get<TransformTree>().GetGlobalTransform(spawner->GetParentEntity().mTransformId);
	glm::vec3 velocityTransformed = transform.Rotation.Rotate(velocity);

	TransformEntity* transformEntity = mWorld.CreateEntity<TransformEntity>(nullptr, transform);
	mWorld.CreateEntity<DynamicRigidBodyEntity>(transformEntity, mSetup.BallPhysicsShape, mSetup.BallPhysicsMaterial, false, velocityTransformed);
	mWorld.CreateEntity<StaticMeshEntity>(transformEntity, mSetup.BallMesh, mSetup.BallTexture);
}
