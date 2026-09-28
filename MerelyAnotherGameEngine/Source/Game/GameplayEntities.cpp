#include "Framework/GameWorld.h"
#include "Game/GameplayEntities.h"

REGISTER_ENTITY_FACTORY_FUNCTION(CapsuleMoverEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(BallSpawnerEntity);

CapsuleMoverEntity::CapsuleMoverEntity(CapsuleMoverEntity::Setup const& inSetup)
	: ChildGameEntity(inSetup), mInputCodeNegative(inSetup.InputCodeNegative), mInputCodePositive(inSetup.InputCodePositive) {}

BallSpawnerEntity::BallSpawnerEntity(BallSpawnerEntity::Setup const& inSetup)
	: ChildGameEntity(inSetup), mSpawnVelocity(inSetup.Velocity), mSpawnVelocityVariance(inSetup.Variance) {}
