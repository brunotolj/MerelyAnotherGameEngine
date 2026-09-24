#include "Framework/GameWorld.h"
#include "Game/GameplayEntities.h"

CapsuleMoverEntity::CapsuleMoverEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, i32 inInputCodeNegative, i32 inInputCodePositive)
	: ChildGameEntity(inWorld, inTypeIndex, inParentEntity), mInputCodeNegative(inInputCodeNegative), mInputCodePositive(inInputCodePositive)
{
	if (TransformTree* transformTree = mWorld.GetComponent<TransformTree>())
	{
		mOriginalTransform = transformTree->GetGlobalTransform(GetParentEntity().mTransformId);
	}
}

EntityFactoryFunction CapsuleMoverEntityFactoryFunction("CapsuleMoverEntity", [](GameWorld& inWorld, GameEntity* inParentEntity, PropertyContainer const& inProperties)
{
	auto getProperty = [&inProperties](mage::StringView inPropertyName) { return inProperties.contains(inPropertyName) ? inProperties.at(inPropertyName) : ""; };
	
	i32 inputKeyNegative = mage::ParseNumber<i32>(getProperty("inputKeyNegative"));
	i32 inputKeyPositive = mage::ParseNumber<i32>(getProperty("inputKeyPositive"));
	
	return inWorld.CreateEntity<CapsuleMoverEntity>(inParentEntity, inputKeyNegative, inputKeyPositive);
});

BallSpawnerEntity::BallSpawnerEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, glm::vec3 inSpawnVelocity, glm::vec3 inSpawnVelocityVariance)
	: ChildGameEntity(inWorld, inTypeIndex, inParentEntity), mSpawnVelocity(inSpawnVelocity), mSpawnVelocityVariance(inSpawnVelocityVariance)
{
}

EntityFactoryFunction BallSpawnerEntityFactoryFunction("BallSpawnerEntity", [](GameWorld& inWorld, GameEntity* inParentEntity, PropertyContainer const& inProperties)
{
	auto getProperty = [&inProperties](mage::StringView inPropertyName) { return inProperties.contains(inPropertyName) ? inProperties.at(inPropertyName) : ""; };

	glm::vec3 velocity
	{
		mage::ParseNumber<f32>(getProperty("velocity.X")),
		mage::ParseNumber<f32>(getProperty("velocity.Y")),
		mage::ParseNumber<f32>(getProperty("velocity.Z"))
	};

	glm::vec3 variance
	{
		mage::ParseNumber<f32>(getProperty("variance.X")),
		mage::ParseNumber<f32>(getProperty("variance.Y")),
		mage::ParseNumber<f32>(getProperty("variance.Z"))
	};

	return inWorld.CreateEntity<BallSpawnerEntity>(inParentEntity, velocity, variance);
});
