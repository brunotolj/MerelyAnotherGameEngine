#include "Engine/Engine.h"
#include "Framework/Entity.h"
#include "Framework/GameWorld.h"
#include "Framework/Systems/PhysicsSystem.h"

std::unordered_map<mage::String, EntityFactoryCallback> gEntityFactoryFunctions;

GameEntity::~GameEntity()
{
	mage_check(mChildEntities.GetSize() == 0);

	if (mParentEntity)
		mParentEntity->RemoveChildEntity(this);
}

void GameEntity::MarkDestroyed()
{
	if (mIsDestoryed)
		return;

	for (GameEntity* child : mChildEntities)
		child->MarkDestroyed();

	mIsDestoryed = true;
}

GameEntity::GameEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity)
	: mWorld(inWorld), mTypeIndex(inTypeIndex), mParentEntity(inParentEntity)
{
	if (mParentEntity)
		mParentEntity->AddChildEntity(this);
}

void GameEntity::AddChildEntity(GameEntity* inChildEntity)
{
	mChildEntities.Add(inChildEntity);

	if (mIsDestoryed)
	{
		mage_ensure(false);
		inChildEntity->MarkDestroyed();
	}
}

void GameEntity::RemoveChildEntity(GameEntity* inChildEntity)
{
	mChildEntities.Remove(inChildEntity);
}

TransformEntity::TransformEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity,
	mage::Transform inInitialTransform, TransformTreeEntryId inTransformParentId)
	: GameEntity(inWorld, inTypeIndex, inParentEntity)
{
	if (TransformTree* transformTree = mWorld.GetComponent<TransformTree>())
	{
		mTransformId = transformTree->AddEntry(inInitialTransform, inTransformParentId);
	}
	else
	{
		mTransformId = mage::InvalidIndex;
		mage_check(false);
	}
}

TransformEntity::~TransformEntity()
{
	if (TransformTree* transformTree = mWorld.GetComponent<TransformTree>())
	{
		transformTree->RemoveEntry(mTransformId);
		mTransformId = mage::InvalidIndex;
	}
}

EntityFactoryFunction TransformEntityFactoryFunction("TransformEntity", [](GameWorld& inWorld, GameEntity* inParentEntity, PropertyContainer const& inProperties)
{
	auto getProperty = [&inProperties](mage::StringView inPropertyName) { return inProperties.contains(inPropertyName) ? inProperties.at(inPropertyName) : ""; };

	glm::vec3 position
	{
		mage::ParseNumber<f32>(getProperty("position.X")),
		mage::ParseNumber<f32>(getProperty("position.Y")),
		mage::ParseNumber<f32>(getProperty("position.Z"))
	};

	glm::vec3 rotationAxis
	{
		mage::ParseNumber<f32>(getProperty("rotationAxis.X")),
		mage::ParseNumber<f32>(getProperty("rotationAxis.Y")),
		mage::ParseNumber<f32>(getProperty("rotationAxis.Z"))
	};

	f32 rotationAngle = glm::radians(mage::ParseNumber<f32>(getProperty("rotationAngle")));

	AssetHandle<Texture> texture = gEngine->mAssetManager.GetHandle<Texture>(getProperty("texture"));

	return inWorld.CreateEntity<TransformEntity>(inParentEntity, mage::Transform{ position, mage::Rotor(rotationAxis, rotationAngle) });
});

StaticMeshEntity::StaticMeshEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity,
	AssetHandle<StaticMesh> inMesh, AssetHandle<Texture> inTexture)
	: ChildGameEntity(inWorld, inTypeIndex, inParentEntity), mMesh(inMesh), mTexture(inTexture)
{
}

EntityFactoryFunction StaticMeshEntityFactoryFunction("StaticMeshEntity", [](GameWorld& inWorld, GameEntity* inParentEntity, PropertyContainer const& inProperties)
{
	auto getProperty = [&inProperties](mage::StringView inPropertyName) { return inProperties.contains(inPropertyName) ? inProperties.at(inPropertyName) : ""; };

	AssetHandle<StaticMesh> mesh = gEngine->mAssetManager.GetHandle<StaticMesh>(getProperty("mesh"));
	AssetHandle<Texture> texture = gEngine->mAssetManager.GetHandle<Texture>(getProperty("texture"));

	return inWorld.CreateEntity<StaticMeshEntity>(inParentEntity, mesh, texture);
});

StaticRigidBodyEntity::StaticRigidBodyEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity,
	AssetHandle<PhysicsShape> inShape, AssetHandle<PhysicsMaterial> inMaterial)
	: ChildGameEntity(inWorld, inTypeIndex, inParentEntity), mShape(inShape), mMaterial(inMaterial)
{
	mPhysicsActor = mWorld.GetComponent<PhysicsSystem>()->CreateStaticRigidBody(
		GetParentEntity().mTransformId, mShape, mMaterial);
}

StaticRigidBodyEntity::~StaticRigidBodyEntity()
{
	mWorld.GetComponent<PhysicsSystem>()->RemoveActor(mPhysicsActor);
	mPhysicsActor = nullptr;
}

EntityFactoryFunction StaticRigidBodyFactoryFunction("StaticRigidBodyEntity", [](GameWorld& inWorld, GameEntity* inParentEntity, PropertyContainer const& inProperties)
{
	auto getProperty = [&inProperties](mage::StringView inPropertyName) { return inProperties.contains(inPropertyName) ? inProperties.at(inPropertyName) : ""; };

	AssetHandle<PhysicsShape> shape = gEngine->mAssetManager.GetHandle<PhysicsShape>(getProperty("shape"));
	AssetHandle<PhysicsMaterial> material = gEngine->mAssetManager.GetHandle<PhysicsMaterial>(getProperty("material"));

	return inWorld.CreateEntity<StaticRigidBodyEntity>(inParentEntity, shape, material);
});

DynamicRigidBodyEntity::DynamicRigidBodyEntity(
	GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, AssetHandle<PhysicsShape> inShape,
	AssetHandle<PhysicsMaterial> inMaterial, bool inIsKinematic, glm::vec3 inLinearVelocity, glm::vec3 inAngularVelocity)
	: ChildGameEntity(inWorld, inTypeIndex, inParentEntity), mShape(inShape), mMaterial(inMaterial), mIsKinematic(inIsKinematic)
{
	mPhysicsActor = mWorld.GetComponent<PhysicsSystem>()->CreateDynamicRigidBody(
		GetParentEntity().mTransformId, mShape, mMaterial, mIsKinematic, inLinearVelocity, inAngularVelocity);
}

DynamicRigidBodyEntity::~DynamicRigidBodyEntity()
{
	mWorld.GetComponent<PhysicsSystem>()->RemoveActor(mPhysicsActor);
	mPhysicsActor = nullptr;
}

EntityFactoryFunction DynamicRigidBodyFactoryFunction("DynamicRigidBodyEntity", [](GameWorld& inWorld, GameEntity* inParentEntity, PropertyContainer const& inProperties)
{
	auto getProperty = [&inProperties](mage::StringView inPropertyName) { return inProperties.contains(inPropertyName) ? inProperties.at(inPropertyName) : ""; };

	AssetHandle<PhysicsShape> shape = gEngine->mAssetManager.GetHandle<PhysicsShape>(getProperty("shape"));
	AssetHandle<PhysicsMaterial> material = gEngine->mAssetManager.GetHandle<PhysicsMaterial>(getProperty("material"));
	bool isKinematic = mage::ParseNumber<bool>(getProperty("isKinematic"));

	glm::vec3 linearVelocity
	{
		mage::ParseNumber<f32>(getProperty("linearVelocity.X")),
		mage::ParseNumber<f32>(getProperty("linearVelocity.Y")),
		mage::ParseNumber<f32>(getProperty("linearVelocity.Z"))
	};

	glm::vec3 angularVelocity
	{
		mage::ParseNumber<f32>(getProperty("angularVelocity.X")),
		mage::ParseNumber<f32>(getProperty("angularVelocity.Y")),
		mage::ParseNumber<f32>(getProperty("angularVelocity.Z"))
	};

	return inWorld.CreateEntity<DynamicRigidBodyEntity>(inParentEntity, shape, material, isKinematic, linearVelocity, angularVelocity);
});

SpriteEntity::SpriteEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, glm::vec2 inScreenCoordsMin,
	glm::vec2 inScreenCoordsMax, glm::vec2 inTextureCoordsMin, glm::vec2 inTextureCoordsMax, AssetHandle<Texture> inTexture)
	: GameEntity(inWorld, inTypeIndex, inParentEntity), mScreenCoordsMin(inScreenCoordsMin), mScreenCoordsMax(inScreenCoordsMax),
	mTextureCoordsMin(inTextureCoordsMin), mTextureCoordsMax(inTextureCoordsMax), mTexture(inTexture)
{
}

EntityFactoryFunction SpriteEntityFactoryFunction("SpriteEntity", [](GameWorld& inWorld, GameEntity* inParentEntity, PropertyContainer const& inProperties)
{
	auto getProperty = [&inProperties](mage::StringView inPropertyName) { return inProperties.contains(inPropertyName) ? inProperties.at(inPropertyName) : ""; };

	glm::vec2 screenCoordsMin = { mage::ParseNumber<f32>(getProperty("screenCoords.Min.X")), mage::ParseNumber<f32>(getProperty("screenCoords.Min.Y")) };
	glm::vec2 screenCoordsMax = { mage::ParseNumber<f32>(getProperty("screenCoords.Max.X")), mage::ParseNumber<f32>(getProperty("screenCoords.Max.Y")) };
	glm::vec2 textureCoordsMin = { mage::ParseNumber<f32>(getProperty("textureCoords.Min.X")), mage::ParseNumber<f32>(getProperty("textureCoords.Min.Y")) };
	glm::vec2 textureCoordsMax = { mage::ParseNumber<f32>(getProperty("textureCoords.Max.X"), 1.0f), mage::ParseNumber<f32>(getProperty("textureCoords.Max.Y"), 1.0f) };
	AssetHandle<Texture> texture = gEngine->mAssetManager.GetHandle<Texture>(getProperty("texture"));

	return inWorld.CreateEntity<SpriteEntity>(inParentEntity, screenCoordsMin, screenCoordsMax, textureCoordsMin, textureCoordsMax, texture);
});

TextEntity::TextEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, mage::StringView inText,
	glm::vec4 inColor, glm::vec2 inScreenPosition, f32 inScale, AssetHandle<Font> inFont)
	: GameEntity(inWorld, inTypeIndex, inParentEntity), mText(inText), mColor(inColor), mScreenPosition(inScreenPosition),
	mScale(inScale), mFont(inFont)
{
}

EntityFactoryFunction TextEntityFactoryFunction("TextEntity", [](GameWorld& inWorld, GameEntity* inParentEntity, PropertyContainer const& inProperties)
{
	auto getProperty = [&inProperties](mage::StringView inPropertyName) -> mage::StringView { if (inProperties.contains(inPropertyName)) return inProperties.at(inPropertyName); return ""; };

	mage::StringView text = getProperty("text");
	glm::vec4 color
	{
		mage::ParseNumber<f32>(getProperty("color.R")),
		mage::ParseNumber<f32>(getProperty("color.G")),
		mage::ParseNumber<f32>(getProperty("color.B")),
		mage::ParseNumber<f32>(getProperty("color.A"), 1.0f)
	};
	glm::vec2 screenPosition = { mage::ParseNumber<f32>(getProperty("position.X")), mage::ParseNumber<f32>(getProperty("position.Y")) };
	f32 scale = mage::ParseNumber<f32>(getProperty("scale"));
	AssetHandle<Font> font = gEngine->mAssetManager.GetHandle<Font>(getProperty("font"));

	return inWorld.CreateEntity<TextEntity>(inParentEntity, text, color, screenPosition, scale, font);
});

EntityFactoryFunction FreeMoveTargetEntityFactoryFunction("FreeMoveTargetEntity", [](GameWorld& inWorld, GameEntity* inParentEntity, PropertyContainer const& inProperties)
{
	return inWorld.CreateEntity<FreeMoveTargetEntity>(inParentEntity);
});

EntityFactoryFunction CameraEntityFactoryFunction("CameraEntity", [](GameWorld& inWorld, GameEntity* inParentEntity, PropertyContainer const& inProperties)
{
	return inWorld.CreateEntity<CameraEntity>(inParentEntity);
});
