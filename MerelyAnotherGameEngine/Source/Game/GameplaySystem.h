#pragma once

#include "Assets/PhysicsMaterial.h"
#include "Assets/PhysicsShape.h"
#include "Assets/StaticMesh.h"
#include "Assets/Texture.h"
#include "Framework/GameWorld.h"
#include "Framework/TransformTree.h"

class CapsuleMoveSystem : public GameSystemWithPrerequisites<TransformTree>
{
public:
	CapsuleMoveSystem(GameWorld& inWorld);

	virtual void GetEntityCallbackTypes(mage::Array<std::type_index>& outTypes) const override;

	virtual void OnEntityCreated(GameEntity* inEntity, GameEntitySetup const* inSetup, std::type_index inType) override;
	virtual void OnEntityDestroyed(GameEntity* inEntity, std::type_index inType) override;

	virtual void Update(f32 inDeltaTime) override;

	f32 HalfSpan = 0.0f;
	f32 MaxSpeed = 0.0f;
	f32 Acceleration = 0.0f;
	f32 Deceleration = 0.0f;
};

template <>
struct Property<CapsuleMoveSystem> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(CapsuleMoveSystem, HalfSpan);
		inOutContainer.AddProperty(CapsuleMoveSystem, MaxSpeed);
		inOutContainer.AddProperty(CapsuleMoveSystem, Acceleration);
		inOutContainer.AddProperty(CapsuleMoveSystem, Deceleration);
	}
};

class BallSpawnerSystem : public GameSystemWithPrerequisites<TransformTree>
{
public:
	BallSpawnerSystem(GameWorld& inWorld);

	virtual void Update(f32 inDeltaTime) override;

	u32 BallCounter = 0;
	f32 SpawnInterval = 0.0f;
	AssetHandle<PhysicsShape> PhysicsShape = nullptr;
	AssetHandle<PhysicsMaterial> PhysicsMaterial = nullptr;
	AssetHandle<StaticMesh> Mesh = nullptr;
	AssetHandle<Texture> Texture = nullptr;

private:
	void SpawnBall();

	f32 mBallSpawnTime = 0.0f;
};

template <>
struct Property<BallSpawnerSystem> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(BallSpawnerSystem, SpawnInterval);
		inOutContainer.AddProperty(BallSpawnerSystem, PhysicsShape);
		inOutContainer.AddProperty(BallSpawnerSystem, PhysicsMaterial);
		inOutContainer.AddProperty(BallSpawnerSystem, Mesh);
		inOutContainer.AddProperty(BallSpawnerSystem, Texture);
	}
};

class GameScoreSystem : public GameSystem
{
public:
	GameScoreSystem(GameWorld& inWorld);

	virtual void GetEntityCallbackTypes(mage::Array<std::type_index>& outTypes) const override;

	virtual void OnEntityCreated(GameEntity* inEntity, GameEntitySetup const* inSetup, std::type_index inType) override;
	virtual void OnEntityDestroyed(GameEntity* inEntity, std::type_index inType) override;

	virtual void Update(f32 inDeltaTime) override;

	u32 InitialScore = 0;
};

template <>
struct Property<GameScoreSystem> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(GameScoreSystem, InitialScore);
	}
};
