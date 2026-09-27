#include "Framework/GameWorld.h"
#include "Game/GameplayEntities.h"

REGISTER_ENTITY_FACTORY_FUNCTION(CapsuleMoverEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(BallSpawnerEntity);

CapsuleMoverEntity::CapsuleMoverEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, CapsuleMoverEntity::Setup const& inSetup)
	: ChildGameEntity(inWorld, inTypeIndex, inParentEntity), mInputCodeNegative(inSetup.InputCodeNegative), mInputCodePositive(inSetup.InputCodePositive)
{
	if (TransformTree* transformTree = mWorld.GetComponent<TransformTree>())
	{
		mOriginalTransform = transformTree->GetGlobalTransform(GetParentEntity().mTransformId);
	}
}

BallSpawnerEntity::BallSpawnerEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, BallSpawnerEntity::Setup const& inSetup)
	: ChildGameEntity(inWorld, inTypeIndex, inParentEntity), mSpawnVelocity(inSetup.Velocity), mSpawnVelocityVariance(inSetup.Variance)
{
}
