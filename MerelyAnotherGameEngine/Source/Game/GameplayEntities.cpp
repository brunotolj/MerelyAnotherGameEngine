#include "Framework/GameWorld.h"
#include "Game/GameplayEntities.h"

REGISTER_ENTITY_FACTORY_FUNCTION(CapsuleMoverEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(BallSpawnerEntity);

CapsuleMoverEntity::CapsuleMoverEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, CapsuleMoverEntity::Setup const& inSetup)
	: ChildGameEntity(inTypeIndex, inParentEntity), mInputCodeNegative(inSetup.InputCodeNegative), mInputCodePositive(inSetup.InputCodePositive) {}

BallSpawnerEntity::BallSpawnerEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, BallSpawnerEntity::Setup const& inSetup)
	: ChildGameEntity(inTypeIndex, inParentEntity), mSpawnVelocity(inSetup.Velocity), mSpawnVelocityVariance(inSetup.Variance) {}
