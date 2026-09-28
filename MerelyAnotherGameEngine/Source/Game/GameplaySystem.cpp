#include "Engine/Engine.h"
#include "Game/GameplayEntities.h"
#include "Game/GameplaySystem.h"

REGISTER_WORLD_COMPONENT_FACTORY_FUNCTION(GameplaySystem);

GameplaySystem::GameplaySystem(GameWorld& inWorld) : GameSystemWithPrerequisites(inWorld)
{
}

void GameplaySystem::GetEntityCallbackTypes(mage::Array<std::type_index>& outTypes) const
{
	outTypes.AddConstruct(typeid(CapsuleMoverEntity));
}

void GameplaySystem::OnEntityCreated(GameEntity* inEntity, GameEntitySetup const* inSetup, std::type_index inType)
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

void GameplaySystem::OnEntityDestroyed(GameEntity* inEntity, std::type_index inType)
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

void GameplaySystem::Update(f32 inDeltaTime)
{
	if (BallSpawnInterval > 0.0f)
	{
		mBallSpawnTime += inDeltaTime;
		while (mBallSpawnTime > BallSpawnInterval)
		{
			mBallSpawnTime -= BallSpawnInterval;
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

	glm::vec3 axis;
	f32 angle;
	transform.Rotation.GetAxisAndAngle(axis, angle);

	TransformEntity::Setup transformSetup{ .Position = transform.Position, .RotationAxis = axis, .RotationAngle = angle };
	TransformEntity* transformEntity = mWorld.CreateEntity<TransformEntity>("", nullptr, transformSetup);

	DynamicRigidBodyEntity::Setup rigidBodySetup{ .Shape = BallPhysicsShape, .Material = BallPhysicsMaterial, .LinearVelocity = velocityTransformed };
	mWorld.CreateEntity<DynamicRigidBodyEntity>("", transformEntity, rigidBodySetup);

	StaticMeshEntity::Setup staticMeshSetup{ .Mesh = BallMesh, .Texture = BallTexture };
	mWorld.CreateEntity<StaticMeshEntity>("", transformEntity, staticMeshSetup);
}
