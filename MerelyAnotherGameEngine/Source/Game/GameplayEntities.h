#pragma once

#include "Framework/Entity.h"

class CapsuleMoverEntity : public ChildGameEntity<TransformEntity>
{
public:
	CapsuleMoverEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, i32 inInputCodeNegative, i32 inInputCodePositive);

	void SetTransformId(TransformTreeEntryId inTransformId);

	mage::Transform mOriginalTransform;
	f32 mPosition = 0.0f;
	f32 mSpeed = 0.0f;
	i32 mInputCodeNegative;
	i32 mInputCodePositive;
};

class BallSpawnerEntity : public ChildGameEntity<TransformEntity>
{
public:
	BallSpawnerEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, glm::vec3 inSpawnVelocity, glm::vec3 inSpawnVelocityVariance);

	glm::vec3 mSpawnVelocity;
	glm::vec3 mSpawnVelocityVariance;
};
