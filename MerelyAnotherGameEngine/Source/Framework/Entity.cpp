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

TransformEntity::TransformEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, TransformEntity::Setup const& inSetup)
	: GameEntity(inWorld, inTypeIndex, inParentEntity)
{
	if (TransformTree* transformTree = mWorld.GetComponent<TransformTree>())
	{
		mTransformId = transformTree->AddEntry({ inSetup.Position, mage::Rotor(inSetup.RotationAxis, glm::radians(inSetup.RotationAngle)) }, inSetup.TransformParentId);
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

StaticMeshEntity::StaticMeshEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, StaticMeshEntity::Setup const& inSetup)
	: ChildGameEntity(inWorld, inTypeIndex, inParentEntity), mMesh(inSetup.Mesh), mTexture(inSetup.Texture)
{
}

StaticRigidBodyEntity::StaticRigidBodyEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, StaticRigidBodyEntity::Setup const& inSetup)
	: ChildGameEntity(inWorld, inTypeIndex, inParentEntity), mShape(inSetup.Shape), mMaterial(inSetup.Material)
{
	mPhysicsActor = mWorld.GetComponent<PhysicsSystem>()->CreateStaticRigidBody(GetParentEntity().mTransformId, mShape, mMaterial);
}

StaticRigidBodyEntity::~StaticRigidBodyEntity()
{
	mWorld.GetComponent<PhysicsSystem>()->RemoveActor(mPhysicsActor);
	mPhysicsActor = nullptr;
}

DynamicRigidBodyEntity::DynamicRigidBodyEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, DynamicRigidBodyEntity::Setup const& inSetup)
	: ChildGameEntity(inWorld, inTypeIndex, inParentEntity), mShape(inSetup.Shape), mMaterial(inSetup.Material), mIsKinematic(inSetup.IsKinematic)
{
	mPhysicsActor = mWorld.GetComponent<PhysicsSystem>()->CreateDynamicRigidBody(
		GetParentEntity().mTransformId, mShape, mMaterial, mIsKinematic, inSetup.LinearVelocity, inSetup.AngularVelocity);
}

DynamicRigidBodyEntity::~DynamicRigidBodyEntity()
{
	mWorld.GetComponent<PhysicsSystem>()->RemoveActor(mPhysicsActor);
	mPhysicsActor = nullptr;
}

SpriteEntity::SpriteEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, SpriteEntity::Setup const& inSetup)
	: GameEntity(inWorld, inTypeIndex, inParentEntity), mScreenCoordsMin(inSetup.ScreenCoordsMin), mScreenCoordsMax(inSetup.ScreenCoordsMax),
	mTextureCoordsMin(inSetup.TextureCoordsMin), mTextureCoordsMax(inSetup.TextureCoordsMax), mTexture(inSetup.Texture)
{
}

TextEntity::TextEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, TextEntity::Setup const& inSetup)
	: GameEntity(inWorld, inTypeIndex, inParentEntity), mText(inSetup.Text), mColor(inSetup.Color),
	mScreenPosition(inSetup.ScreenPosition), mScale(inSetup.Scale), mFont(inSetup.Font)
{
}
