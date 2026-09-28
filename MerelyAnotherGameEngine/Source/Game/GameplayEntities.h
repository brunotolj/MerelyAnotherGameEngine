#pragma once

#include "Framework/Entity.h"

class CapsuleMoverEntity : public ChildGameEntity<TransformEntity>
{
public:
	struct Setup : public GameEntitySetup
	{
		i32 InputCodeNegative;
		i32 InputCodePositive;
	};

	CapsuleMoverEntity(Setup const& inSetup);

	mage::Transform mOriginalTransform;
	f32 mPosition = 0.0f;
	f32 mSpeed = 0.0f;
	i32 mInputCodeNegative = 0;
	i32 mInputCodePositive = 0;
};

template<>
struct Property<CapsuleMoverEntity::Setup> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(CapsuleMoverEntity::Setup, InputCodeNegative);
		inOutContainer.AddProperty(CapsuleMoverEntity::Setup, InputCodePositive);
	}
};

class BallSpawnerEntity : public ChildGameEntity<TransformEntity>
{
public:
	struct Setup : public GameEntitySetup
	{
		glm::vec3 Velocity{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Variance{ 0.0f, 0.0f, 0.0f };
	};

	BallSpawnerEntity(Setup const& inSetup);

	glm::vec3 mSpawnVelocity;
	glm::vec3 mSpawnVelocityVariance;
};

template<>
struct Property<BallSpawnerEntity::Setup> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(BallSpawnerEntity::Setup, Velocity);
		inOutContainer.AddProperty(BallSpawnerEntity::Setup, Variance);
	}
};

class PlayerScoreEntity : public ChildGameEntity<TextEntity>
{
public:
	struct Setup : public GameEntitySetup
	{
		u32 PlayerIndex = mage::InvalidIndex;
	};

	PlayerScoreEntity(Setup const& inSetup);

	u32 mPlayerIndex;
	u32 mScore = 0;
};

template<>
struct Property<PlayerScoreEntity::Setup> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(PlayerScoreEntity::Setup, PlayerIndex);
	}
};

class PlayerTriggerTrackerEntity : public ChildGameEntity<StaticTriggerVolumeEntity>
{
public:
	struct Setup : public GameEntitySetup
	{
		u32 PlayerIndex = mage::InvalidIndex;
	};

	PlayerTriggerTrackerEntity(Setup const& inSetup);

	u32 mPlayerIndex;
	mage::Array<mage::String> mProcessedOverlaps;
};

template<>
struct Property<PlayerTriggerTrackerEntity::Setup> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(PlayerTriggerTrackerEntity::Setup, PlayerIndex);
	}
};
