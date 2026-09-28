#include "Engine/Engine.h"
#include "Game/GameplayEntities.h"
#include "Game/GameplaySystem.h"

REGISTER_WORLD_COMPONENT_FACTORY_FUNCTION(CapsuleMoveSystem);
REGISTER_WORLD_COMPONENT_FACTORY_FUNCTION(BallSpawnerSystem);
REGISTER_WORLD_COMPONENT_FACTORY_FUNCTION(GameScoreSystem);

CapsuleMoveSystem::CapsuleMoveSystem(GameWorld& inWorld) : GameSystemWithPrerequisites(inWorld)
{
}

void CapsuleMoveSystem::GetEntityCallbackTypes(mage::Array<std::type_index>& outTypes) const
{
	outTypes.AddConstruct(typeid(CapsuleMoverEntity));
}

void CapsuleMoveSystem::OnEntityCreated(GameEntity* inEntity, GameEntitySetup const* inSetup, std::type_index inType)
{
	if (inType == typeid(CapsuleMoverEntity))
	{
		CapsuleMoverEntity* entity = (CapsuleMoverEntity*)(inEntity);
		CapsuleMoverEntity::Setup const* setup = (CapsuleMoverEntity::Setup const*)(inSetup);

		entity->mOriginalTransform = Get<TransformTree>().GetGlobalTransform(entity->GetParentEntity().mTransformId);
	}
	else
	{
		mage_check(false);
	}
}

void CapsuleMoveSystem::OnEntityDestroyed(GameEntity* inEntity, std::type_index inType)
{
	if (inType == typeid(CapsuleMoverEntity))
	{
		CapsuleMoverEntity* entity = (CapsuleMoverEntity*)(inEntity);
	}
	else
	{
		mage_check(false);
	}
}

void CapsuleMoveSystem::Update(f32 inDeltaTime)
{
	for (CapsuleMoverEntity* capsuleMoverEntity : mWorld.GetEntities<CapsuleMoverEntity>())
	{
		f32 input = 0.0f;
		if (gEngine->mInputHandler.IsKeyPressed(capsuleMoverEntity->mInputCodeNegative)) input -= 1.0f;
		if (gEngine->mInputHandler.IsKeyPressed(capsuleMoverEntity->mInputCodePositive)) input += 1.0f;

		f32 remainingTime = inDeltaTime;
		f32 movement = 0.0f;

		if (remainingTime > 0.0f && capsuleMoverEntity->mSpeed != 0.0f && input * capsuleMoverEntity->mSpeed <= 0.0f)
		{
			f32 decelTime = std::fabsf(capsuleMoverEntity->mSpeed) / Deceleration;
			if (decelTime > remainingTime)
			{
				f32 deltaSpeed = capsuleMoverEntity->mSpeed / std::fabsf(capsuleMoverEntity->mSpeed) * Deceleration * remainingTime;
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
			f32 accelTime = (MaxSpeed - input * capsuleMoverEntity->mSpeed) / Acceleration;
			if (accelTime > remainingTime)
			{
				f32 deltaSpeed = input * Acceleration * remainingTime;
				movement += (capsuleMoverEntity->mSpeed + 0.5f * deltaSpeed) * remainingTime;
				capsuleMoverEntity->mSpeed += deltaSpeed;
				remainingTime = 0.0f;
			}
			else
			{
				movement += 0.5f * (capsuleMoverEntity->mSpeed + input * MaxSpeed) * accelTime;
				capsuleMoverEntity->mSpeed = input * MaxSpeed;
				remainingTime -= accelTime;
			}
		}

		movement += capsuleMoverEntity->mSpeed * remainingTime;

		capsuleMoverEntity->mPosition += movement;
		if (capsuleMoverEntity->mPosition > HalfSpan)
		{
			capsuleMoverEntity->mPosition = HalfSpan;
			capsuleMoverEntity->mSpeed = 0.0f;
		}
		else if (capsuleMoverEntity->mPosition < -HalfSpan)
		{
			capsuleMoverEntity->mPosition = -HalfSpan;
			capsuleMoverEntity->mSpeed = 0.0f;
		}

		mage::Transform transform = capsuleMoverEntity->mOriginalTransform;
		transform.Position += capsuleMoverEntity->mPosition * transform.Rotation.Rotate({ 0.0f, -1.0f, 0.0f });
		Get<TransformTree>().SetGlobalTransform(capsuleMoverEntity->GetParentEntity().mTransformId, transform);
	}
}

BallSpawnerSystem::BallSpawnerSystem(GameWorld& inWorld) : GameSystemWithPrerequisites(inWorld)
{
}

void BallSpawnerSystem::Update(f32 inDeltaTime)
{
	if (SpawnInterval > 0.0f)
	{
		mBallSpawnTime += inDeltaTime;
		while (mBallSpawnTime > SpawnInterval)
		{
			mBallSpawnTime -= SpawnInterval;
			SpawnBall();
		}
	}
}

void BallSpawnerSystem::SpawnBall()
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

	glm::vec3 axis;
	f32 angle;
	transform.Rotation.GetAxisAndAngle(axis, angle);

	TransformEntity::Setup transformSetup{ .Position = transform.Position, .RotationAxis = axis, .RotationAngle = angle };
	TransformEntity* transformEntity = mWorld.CreateEntity<TransformEntity>("", nullptr, transformSetup);

	mage::String name = "Ball_";
	name.Append(mage::String().FromNumber(BallCounter++));

	DynamicRigidBodyEntity::Setup rigidBodySetup{ .Shape = PhysicsShape, .Material = PhysicsMaterial, .LinearVelocity = velocityTransformed };
	mWorld.CreateEntity<DynamicRigidBodyEntity>(name, transformEntity, rigidBodySetup);

	StaticMeshEntity::Setup staticMeshSetup{ .Mesh = Mesh, .Texture = Texture };
	mWorld.CreateEntity<StaticMeshEntity>("", transformEntity, staticMeshSetup);
}

GameScoreSystem::GameScoreSystem(GameWorld& inWorld) : GameSystem(inWorld)
{
}

void GameScoreSystem::GetEntityCallbackTypes(mage::Array<std::type_index>& outTypes) const
{
	outTypes.AddConstruct(typeid(PlayerScoreEntity));
}

void GameScoreSystem::OnEntityCreated(GameEntity* inEntity, GameEntitySetup const* inSetup, std::type_index inType)
{
	if (inType == typeid(PlayerScoreEntity))
	{
		PlayerScoreEntity* entity = (PlayerScoreEntity*)(inEntity);
		PlayerScoreEntity::Setup const* setup = (PlayerScoreEntity::Setup const*)(inSetup);

		entity->mScore = InitialScore;
		entity->GetParentEntity().mText.FromNumber(InitialScore);
	}
	else
	{
		mage_check(false);
	}
}

void GameScoreSystem::OnEntityDestroyed(GameEntity* inEntity, std::type_index inType)
{
	if (inType == typeid(PlayerScoreEntity))
	{
		PlayerScoreEntity* entity = (PlayerScoreEntity*)(inEntity);
	}
	else
	{
		mage_check(false);
	}
}

void GameScoreSystem::Update(f32 inDeltaTime)
{
	u32 newOverlapCount[4]{};

	for (PlayerTriggerTrackerEntity* triggerTracker : mWorld.GetEntities<PlayerTriggerTrackerEntity>())
	{
		if (triggerTracker->mPlayerIndex >= 4) continue;

		for (DynamicRigidBodyEntity* overlappedBody : triggerTracker->GetParentEntity().mOverlaps)
		{
			if (triggerTracker->mProcessedOverlaps.Contains(overlappedBody->mName))
				continue;

			triggerTracker->mProcessedOverlaps.Add(overlappedBody->mName);
			newOverlapCount[triggerTracker->mPlayerIndex]++;
		}
	}

	for (PlayerScoreEntity* scoreEntity : mWorld.GetEntities<PlayerScoreEntity>())
	{
		if (scoreEntity->mPlayerIndex >= 4) continue;

		scoreEntity->mScore -= std::min(scoreEntity->mScore, newOverlapCount[scoreEntity->mPlayerIndex]);
		scoreEntity->GetParentEntity().mText.FromNumber(scoreEntity->mScore);
	}
}
