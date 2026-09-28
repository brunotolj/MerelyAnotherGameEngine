#include "Engine/Engine.h"
#include "Framework/Entity.h"
#include "Framework/GameWorld.h"
#include "Framework/Systems/PhysicsSystem.h"

std::unordered_map<mage::String, EntityFactoryCallback> gEntityFactoryFunctions;

REGISTER_ENTITY_FACTORY_FUNCTION(TransformEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(StaticMeshEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(StaticRigidBodyEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(DynamicRigidBodyEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(SpriteEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(TextEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(FreeMoveTargetEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(CameraEntity);

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

GameEntity::GameEntity(std::type_index inTypeIndex, GameEntity* inParentEntity)
	: mTypeIndex(inTypeIndex), mParentEntity(inParentEntity)
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

TransformEntity::TransformEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, TransformEntity::Setup const& inSetup)
	: GameEntity(inTypeIndex, inParentEntity) {}

StaticMeshEntity::StaticMeshEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, StaticMeshEntity::Setup const& inSetup)
	: ChildGameEntity(inTypeIndex, inParentEntity), mMesh(inSetup.Mesh), mTexture(inSetup.Texture) {}

StaticRigidBodyEntity::StaticRigidBodyEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, StaticRigidBodyEntity::Setup const& inSetup)
	: ChildGameEntity(inTypeIndex, inParentEntity), mShape(inSetup.Shape), mMaterial(inSetup.Material) {}

DynamicRigidBodyEntity::DynamicRigidBodyEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, DynamicRigidBodyEntity::Setup const& inSetup)
	: ChildGameEntity(inTypeIndex, inParentEntity), mShape(inSetup.Shape), mMaterial(inSetup.Material), mIsKinematic(inSetup.IsKinematic) {}

SpriteEntity::SpriteEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, SpriteEntity::Setup const& inSetup)
	: GameEntity(inTypeIndex, inParentEntity), mScreenPosition(inSetup.ScreenPosition), mScreenSize(inSetup.ScreenSize), mAnchor(inSetup.Anchor),
	mTextureCoordsMin(inSetup.TextureCoordsMin), mTextureCoordsMax(inSetup.TextureCoordsMax), mTexture(inSetup.Texture) {}

TextEntity::TextEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, TextEntity::Setup const& inSetup)
	: GameEntity(inTypeIndex, inParentEntity), mText(inSetup.Text), mColor(inSetup.Color), mScreenPosition(inSetup.ScreenPosition),
	mJustification(inSetup.Justification), mScale(inSetup.Scale), mFont(inSetup.Font) {}
