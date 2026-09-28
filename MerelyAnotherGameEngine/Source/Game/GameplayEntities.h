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

	CapsuleMoverEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, Setup const& inSetup);

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

	BallSpawnerEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, Setup const& inSetup);

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
