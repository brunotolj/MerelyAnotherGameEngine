#include "Framework/GameWorld.h"
#include "Game/GameplayEntities.h"

PlayerEntity::PlayerEntity(GameWorld& inWorld, std::type_index inTypeIndex, i32 inInputCodeNegative, i32 inInputCodePositive)
	: GameEntity(inWorld, inTypeIndex, nullptr), mInputCodeNegative(inInputCodeNegative), mInputCodePositive(inInputCodePositive)
{
}

void PlayerEntity::SetTransformId(TransformTreeEntryId inTransformId)
{
	mTransformId = inTransformId;

	if (TransformTree* transformTree = mWorld.GetComponent<TransformTree>())
	{
		mOriginalTransform = transformTree->GetGlobalTransform(mTransformId);
	}
}

BallSpawnerEntity::BallSpawnerEntity(GameWorld& inWorld, std::type_index inTypeIndex, TransformEntity& inParentEntity, glm::vec3 inSpawnVelocity,
	glm::vec3 inSpawnVelocityVariance)
	: ChildGameEntity(inWorld, inTypeIndex, inParentEntity), mSpawnVelocity(inSpawnVelocity), mSpawnVelocityVariance(inSpawnVelocityVariance)
{
}
