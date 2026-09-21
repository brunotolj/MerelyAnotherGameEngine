#include "Framework/Entity.h"
#include "Framework/Systems/PhysicsSystem.h"

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

StaticMeshEntity::StaticMeshEntity(GameWorld& inWorld, std::type_index inTypeIndex, TransformEntity& inParentEntity,
	AssetHandle<StaticMesh> inMesh, AssetHandle<Texture> inTexture)
	: ChildGameEntity(inWorld, inTypeIndex, inParentEntity), mMesh(inMesh), mTexture(inTexture)
{
}

StaticRigidBodyEntity::StaticRigidBodyEntity(GameWorld& inWorld, std::type_index inTypeIndex, TransformEntity& inParentEntity,
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

DynamicRigidBodyEntity::DynamicRigidBodyEntity(
	GameWorld& inWorld, std::type_index inTypeIndex, TransformEntity& inParentEntity, AssetHandle<PhysicsShape> inShape,
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

SpriteEntity::SpriteEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, glm::vec2 inScreenCoordsMin,
	glm::vec2 inScreenCoordsMax, glm::vec2 inTextureCoordsMin, glm::vec2 inTextureCoordsMax, AssetHandle<Texture> inTexture)
	: GameEntity(inWorld, inTypeIndex, inParentEntity), mScreenCoordsMin(inScreenCoordsMin), mScreenCoordsMax(inScreenCoordsMax),
	mTextureCoordsMin(inTextureCoordsMin), mTextureCoordsMax(inTextureCoordsMax), mTexture(inTexture)
{
}

TextEntity::TextEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity, mage::StringView inText,
	glm::vec4 inColor, glm::vec2 inScreenPosition, f32 inScale, AssetHandle<Font> inFont)
	: GameEntity(inWorld, inTypeIndex, inParentEntity), mText(inText), mColor(inColor), mScreenPosition(inScreenPosition),
	mScale(inScale), mFont(inFont)
{
}
