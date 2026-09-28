#include "Framework/Entity.h"
#include "Framework/GameWorld.h"

std::unordered_map<mage::String, EntityFactoryCallback> gEntityFactoryFunctions;

REGISTER_ENTITY_FACTORY_FUNCTION(TransformEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(StaticMeshEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(StaticRigidBodyEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(DynamicRigidBodyEntity);
REGISTER_ENTITY_FACTORY_FUNCTION(StaticTriggerVolumeEntity);
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

GameEntity::GameEntity(GameEntitySetup const& inSetup)
	: mName(inSetup.Name), mTypeIndex(inSetup.TypeIndex), mParentEntity(inSetup.ParentEntity)
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

TransformEntity::TransformEntity(TransformEntity::Setup const& inSetup)
	: GameEntity(inSetup) {}

StaticMeshEntity::StaticMeshEntity(StaticMeshEntity::Setup const& inSetup)
	: ChildGameEntity(inSetup), mMesh(inSetup.Mesh), mTexture(inSetup.Texture) {}

StaticRigidBodyEntity::StaticRigidBodyEntity(StaticRigidBodyEntity::Setup const& inSetup)
	: ChildGameEntity(inSetup), mShape(inSetup.Shape), mMaterial(inSetup.Material) {}

DynamicRigidBodyEntity::DynamicRigidBodyEntity(DynamicRigidBodyEntity::Setup const& inSetup)
	: ChildGameEntity(inSetup), mShape(inSetup.Shape), mMaterial(inSetup.Material), mIsKinematic(inSetup.IsKinematic) {}

StaticTriggerVolumeEntity::StaticTriggerVolumeEntity(StaticTriggerVolumeEntity::Setup const& inSetup)
	: ChildGameEntity(inSetup), mShape(inSetup.Shape) {}

SpriteEntity::SpriteEntity(SpriteEntity::Setup const& inSetup)
	: GameEntity(inSetup), mScreenPosition(inSetup.ScreenPosition), mScreenSize(inSetup.ScreenSize), mAnchor(inSetup.Anchor),
	mTextureCoordsMin(inSetup.TextureCoordsMin), mTextureCoordsMax(inSetup.TextureCoordsMax), mTexture(inSetup.Texture) {}

TextEntity::TextEntity(TextEntity::Setup const& inSetup)
	: GameEntity(inSetup), mText(inSetup.Text), mColor(inSetup.Color), mScreenPosition(inSetup.ScreenPosition),
	mJustification(inSetup.Justification), mScale(inSetup.Scale), mFont(inSetup.Font) {}
