#pragma once

#include "Assets/Font.h"
#include "Assets/PhysicsMaterial.h"
#include "Assets/PhysicsShape.h"
#include "Assets/StaticMesh.h"
#include "Assets/Texture.h"
#include "Framework/TransformTree.h"

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
	ParentEntityClass& GetParentEntity() const { return (ParentEntityClass&)(*mParentEntity); }

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

class StaticRigidBodyEntity : public ChildGameEntity<TransformEntity>
{
public:
	StaticRigidBodyEntity(
		GameWorld& inWorld,
		std::type_index inTypeIndex,
		TransformEntity& inParentEntity,
		AssetHandle<PhysicsShape> inShape,
		AssetHandle<PhysicsMaterial> inMaterial);

	virtual ~StaticRigidBodyEntity();

	AssetHandle<PhysicsShape> mShape;
	AssetHandle<PhysicsMaterial> mMaterial;
	physx::PxRigidStatic* mPhysicsActor;
};

class DynamicRigidBodyEntity : public ChildGameEntity<TransformEntity>
{
public:
	DynamicRigidBodyEntity(
		GameWorld& inWorld,
		std::type_index inTypeIndex,
		TransformEntity& inParentEntity,
		AssetHandle<PhysicsShape> inShape,
		AssetHandle<PhysicsMaterial> inMaterial,
		bool inIsKinematic,
		glm::vec3 inLinearVelocity = glm::vec3(0.0f),
		glm::vec3 inAngularVelocity = glm::vec3(0.0f));

	virtual ~DynamicRigidBodyEntity();

	AssetHandle<PhysicsShape> mShape;
	AssetHandle<PhysicsMaterial> mMaterial;
	physx::PxRigidDynamic* mPhysicsActor;
	bool mIsKinematic;
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
