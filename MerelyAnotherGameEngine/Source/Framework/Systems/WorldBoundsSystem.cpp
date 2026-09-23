#include "Framework/Systems/WorldBoundsSystem.h"

WorldComponentFactoryFunction WorldBoundsSystemFactoryFunction("WorldBoundsSystem", [](GameWorld& inWorld, PropertyContainer const& inProperties)
{
	auto getProperty = [&inProperties](mage::StringView inPropertyName) { return inProperties.contains(inPropertyName) ? inProperties.at(inPropertyName) : ""; };

	f32 minX = mage::ParseNumber<f32>(getProperty("minX"));
	f32 maxX = mage::ParseNumber<f32>(getProperty("maxX"));
	f32 minY = mage::ParseNumber<f32>(getProperty("minY"));
	f32 maxY = mage::ParseNumber<f32>(getProperty("maxY"));
	f32 minZ = mage::ParseNumber<f32>(getProperty("minZ"));
	f32 maxZ = mage::ParseNumber<f32>(getProperty("maxZ"));

	inWorld.CreateComponent<WorldBoundsSystem>(glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
});

WorldBoundsSystem::WorldBoundsSystem(GameWorld& inWorld, glm::vec3 inBoundsMin, glm::vec3 inBoundsMax)
	: GameSystemWithPrerequisites(inWorld), mBoundsMin(inBoundsMin), mBoundsMax(inBoundsMax)
{
	mage_check(inBoundsMin.x <= inBoundsMax.x);
	mage_check(inBoundsMin.y <= inBoundsMax.y);
	mage_check(inBoundsMin.z <= inBoundsMax.z);
}

void WorldBoundsSystem::Update(f32 inDeltaTime)
{
	for (TransformEntity* transformEntity : mWorld.GetEntities<TransformEntity>())
	{
		TransformTreeEntryId transformId = transformEntity->mTransformId;
		glm::vec3 position = Get<TransformTree>().GetGlobalTransform(transformId).Position;

		bool outOfBounds = false;
		outOfBounds |= position.x < mBoundsMin.x;
		outOfBounds |= position.y < mBoundsMin.y;
		outOfBounds |= position.z < mBoundsMin.z;
		outOfBounds |= position.x > mBoundsMax.x;
		outOfBounds |= position.y > mBoundsMax.y;
		outOfBounds |= position.z > mBoundsMax.z;

		if (outOfBounds)
			transformEntity->MarkDestroyed();
	}
}
