#pragma once

#include "Assets/Font.h"
#include "Assets/StaticMesh.h"
#include "Assets/Texture.h"
#include "Framework/TransformTree.h"
#include "Physics/PhysicsCommon.h"

class GameEntity : public NonCopyable
{
public:
	virtual ~GameEntity();

	void MarkDestroyed();
	bool IsDestroyed() const { return mIsDestoryed; }

	std::type_index mTypeIndex;
	mage::Array<GameEntity*> mChildEntities;

protected:
	GameEntity(GameWorld& inWorld, std::type_index inTypeIndex, GameEntity* inParentEntity);

	void AddChildEntity(GameEntity* inChildEntity);
	void RemoveChildEntity(GameEntity* inChildEntity);

	GameWorld& mWorld;
	GameEntity* mParentEntity;
	bool mIsDestoryed = false;
};

template <typename ParentEntityClass = GameEntity>
class ChildGameEntity : public GameEntity
{
public:
	ParentEntityClass& GetParentEntity() { return reinterpret_cast<ParentEntityClass&>(*mParentEntity); }

protected:
	ChildGameEntity(GameWorld& inWorld, std::type_index inTypeIndex, ParentEntityClass& inParentEntity)
		: GameEntity(inWorld, inTypeIndex, &inParentEntity) {}
};

class TransformEntity : public GameEntity
{
public:
	TransformEntity(
		GameWorld& inWorld,
		std::type_index inTypeIndex,
		GameEntity* inParentEntity,
		mage::Transform inInitialTransform = mage::Transform(),
		TransformTreeEntryId inTransformParentId = mage::InvalidIndex);

	virtual ~TransformEntity();

	TransformTreeEntryId mTransformId;
};

class StaticMeshEntity : public ChildGameEntity<TransformEntity>
{
public:
	StaticMeshEntity(
		GameWorld& inWorld,
		std::type_index inTypeIndex,
		TransformEntity& inParentEntity,
		AssetHandle<StaticMesh> inMesh,
		AssetHandle<Texture> inTexture);

	AssetHandle<StaticMesh> mMesh;
	AssetHandle<Texture> mTexture;
};

class RigidBodyEntity : public ChildGameEntity<TransformEntity>
{
public:
	RigidBodyEntity(
		GameWorld& inWorld,
		std::type_index inTypeIndex,
		TransformEntity& inParentEntity,
		PhysicsRigidBodyParams inRigidBodyParams,
		physx::PxVec3 inLinearVelocity = physx::PxVec3(0.0f),
		physx::PxVec3 inAngularVelocity = physx::PxVec3(0.0f));

	virtual ~RigidBodyEntity();

	PhysicsRigidBodyParams mRigidBodyParams;
	physx::PxRigidActor* mPhysicsActor;
};

class SpriteEntity : public GameEntity
{
public:
	SpriteEntity(
		GameWorld& inWorld,
		std::type_index inTypeIndex,
		GameEntity* inParentEntity,
		glm::vec2 inScreenCoordsMin,
		glm::vec2 inScreenCoordsMax,
		glm::vec2 inTextureCoordsMin,
		glm::vec2 inTextureCoordsMax,
		AssetHandle<Texture> inTexture);

	glm::vec2 mScreenCoordsMin;
	glm::vec2 mScreenCoordsMax;
	glm::vec2 mTextureCoordsMin;
	glm::vec2 mTextureCoordsMax;
	AssetHandle<Texture> mTexture;
};

class TextEntity : public GameEntity
{
public:
	TextEntity(
		GameWorld& inWorld,
		std::type_index inTypeIndex,
		GameEntity* inParentEntity,
		mage::StringView inText,
		glm::vec4 inColor,
		glm::vec2 inScreenPosition,
		f32 inScale,
		AssetHandle<Font> inFont);

	mage::StringView mText;
	glm::vec4 mColor;
	glm::vec2 mScreenPosition;
	f32 mScale;
	AssetHandle<Font> mFont;
};
