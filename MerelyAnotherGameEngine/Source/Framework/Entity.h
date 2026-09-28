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

	static bool IsParentEntityValid(GameEntity* inParentEntity) { return true; }

	void MarkDestroyed();
	bool IsDestroyed() const { return mIsDestoryed; }

	std::type_index mTypeIndex;
	mage::Array<GameEntity*> mChildEntities;

protected:
	GameEntity(std::type_index inTypeIndex, GameEntity* inParentEntity);

	void AddChildEntity(GameEntity* inChildEntity);
	void RemoveChildEntity(GameEntity* inChildEntity);

	GameEntity* mParentEntity;
	bool mIsDestoryed = false;
};

template <typename Type>
concept EntityType = std::derived_from<Type, GameEntity> && !std::same_as<Type, GameEntity>;

struct GameEntitySetup {};

template <typename Type>
concept EntitySetupType = std::derived_from<Type, GameEntitySetup> && !std::same_as<Type, GameEntitySetup>;

template <EntityType ParentEntityClass>
class ChildGameEntity : public GameEntity
{
public:
	ParentEntityClass& GetParentEntity() const { return (ParentEntityClass&)(*mParentEntity); }

	static bool IsParentEntityValid(GameEntity* inParentEntity)
	{
		return inParentEntity && (inParentEntity->mTypeIndex == typeid(ParentEntityClass));
	}

protected:
	ChildGameEntity(std::type_index inTypeIndex, GameEntity* inParentEntity)
		: GameEntity(inTypeIndex, inParentEntity) {}
};

class TransformEntity : public GameEntity
{
public:
	struct Setup : public GameEntitySetup
	{
		glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 RotationAxis{ 0.0f, 0.0f, 0.0f };
		f32 RotationAngle = 0.0f;
		TransformTreeEntryId TransformParentId = mage::InvalidIndex;
	};

	TransformEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, Setup const& inSetup);

	TransformTreeEntryId mTransformId = mage::InvalidIndex;
};

template <>
struct Property<TransformEntity::Setup> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(TransformEntity::Setup, Position);
		inOutContainer.AddProperty(TransformEntity::Setup, RotationAxis);
		inOutContainer.AddProperty(TransformEntity::Setup, RotationAngle);
	}
};

class StaticMeshEntity : public ChildGameEntity<TransformEntity>
{
public:
	struct Setup : public GameEntitySetup
	{
		AssetHandle<StaticMesh> Mesh = nullptr;
		AssetHandle<Texture> Texture = nullptr;
	};

	StaticMeshEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, Setup const& inSetup);

	AssetHandle<StaticMesh> mMesh;
	AssetHandle<Texture> mTexture;
};

template <>
struct Property<StaticMeshEntity::Setup> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(StaticMeshEntity::Setup, Mesh);
		inOutContainer.AddProperty(StaticMeshEntity::Setup, Texture);
	}
};

class StaticRigidBodyEntity : public ChildGameEntity<TransformEntity>
{
public:
	struct Setup : public GameEntitySetup
	{
		AssetHandle<PhysicsShape> Shape = nullptr;
		AssetHandle<PhysicsMaterial> Material = nullptr;
	};

	StaticRigidBodyEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, Setup const& inSetup);

	AssetHandle<PhysicsShape> mShape;
	AssetHandle<PhysicsMaterial> mMaterial;
	physx::PxRigidStatic* mPhysicsActor;
};

template <>
struct Property<StaticRigidBodyEntity::Setup> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(StaticRigidBodyEntity::Setup, Shape);
		inOutContainer.AddProperty(StaticRigidBodyEntity::Setup, Material);
	}
};

class DynamicRigidBodyEntity : public ChildGameEntity<TransformEntity>
{
public:
	struct Setup : public GameEntitySetup
	{
		AssetHandle<PhysicsShape> Shape = nullptr;
		AssetHandle<PhysicsMaterial> Material = nullptr;
		bool IsKinematic = false;
		glm::vec3 LinearVelocity = glm::vec3(0.0f);
		glm::vec3 AngularVelocity = glm::vec3(0.0f);
	};

	DynamicRigidBodyEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, Setup const& inSetup);

	AssetHandle<PhysicsShape> mShape;
	AssetHandle<PhysicsMaterial> mMaterial;
	physx::PxRigidDynamic* mPhysicsActor;
	bool mIsKinematic;
};

template <>
struct Property<DynamicRigidBodyEntity::Setup> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(DynamicRigidBodyEntity::Setup, Shape);
		inOutContainer.AddProperty(DynamicRigidBodyEntity::Setup, Material);
		inOutContainer.AddProperty(DynamicRigidBodyEntity::Setup, IsKinematic);
		inOutContainer.AddProperty(DynamicRigidBodyEntity::Setup, LinearVelocity);
		inOutContainer.AddProperty(DynamicRigidBodyEntity::Setup, AngularVelocity);
	}
};

class StaticTriggerVolumeEntity : public ChildGameEntity<TransformEntity>
{
public:
	struct Setup : public GameEntitySetup
	{
		AssetHandle<PhysicsShape> Shape = nullptr;
	};

	StaticTriggerVolumeEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, Setup const& inSetup);

	AssetHandle<PhysicsShape> mShape;
	physx::PxRigidStatic* mPhysicsActor;
	mage::Array<DynamicRigidBodyEntity*> mOverlaps;
};

template <>
struct Property<StaticTriggerVolumeEntity::Setup> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(StaticTriggerVolumeEntity::Setup, Shape);
	}
};

class SpriteEntity : public GameEntity
{
public:
	struct Setup : public GameEntitySetup
	{
		glm::vec2 ScreenPosition{ 0.0f, 0.0f };
		glm::vec2 ScreenSize{ 100.0f, 100.0f };
		glm::vec2 Anchor{ 0.0f, 0.0f };
		glm::vec2 TextureCoordsMin{ 0.0f, 0.0f };
		glm::vec2 TextureCoordsMax{ 1.0f, 1.0f };
		AssetHandle<Texture> Texture = nullptr;
	};

	SpriteEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, Setup const& inSetup);

	glm::vec2 mScreenPosition;
	glm::vec2 mScreenSize;
	glm::vec2 mAnchor;
	glm::vec2 mTextureCoordsMin;
	glm::vec2 mTextureCoordsMax;
	AssetHandle<Texture> mTexture;
};

template <>
struct Property<SpriteEntity::Setup> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(SpriteEntity::Setup, ScreenPosition);
		inOutContainer.AddProperty(SpriteEntity::Setup, ScreenSize);
		inOutContainer.AddProperty(SpriteEntity::Setup, Anchor);
		inOutContainer.AddProperty(SpriteEntity::Setup, TextureCoordsMin);
		inOutContainer.AddProperty(SpriteEntity::Setup, TextureCoordsMax);
		inOutContainer.AddProperty(SpriteEntity::Setup, Texture);
	}
};

class TextEntity : public GameEntity
{
public:
	struct Setup : public GameEntitySetup
	{
		mage::String Text;
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec2 ScreenPosition{ 0.0f, 0.0f };
		f32 Justification = 0.0f;
		f32 Scale = 1.0f;
		AssetHandle<Font> Font = nullptr;
	};

	TextEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, Setup const& inSetup);

	mage::String mText;
	glm::vec4 mColor;
	glm::vec2 mScreenPosition;
	f32 mJustification;
	f32 mScale;
	AssetHandle<Font> mFont;
};

template <>
struct Property<TextEntity::Setup> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer)
	{
		inOutContainer.AddProperty(TextEntity::Setup, Text);
		inOutContainer.AddProperty(TextEntity::Setup, Color);
		inOutContainer.AddProperty(TextEntity::Setup, ScreenPosition);
		inOutContainer.AddProperty(TextEntity::Setup, Justification);
		inOutContainer.AddProperty(TextEntity::Setup, Scale);
		inOutContainer.AddProperty(TextEntity::Setup, Font);
	}
};

class FreeMoveTargetEntity : public ChildGameEntity<TransformEntity>
{
public:
	struct Setup : public GameEntitySetup {};

	FreeMoveTargetEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, Setup const& inSetup)
		: ChildGameEntity(inTypeIndex, inParentEntity) {}
};

class CameraEntity : public ChildGameEntity<TransformEntity>
{
public:
	struct Setup : public GameEntitySetup {};

	CameraEntity(std::type_index inTypeIndex, GameEntity* inParentEntity, Setup const& inSetup)
		: ChildGameEntity(inTypeIndex, inParentEntity) {}
};

template <EntitySetupType Type>
struct Property<Type> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer) {}
};

using EntityFactoryCallback = std::function<GameEntity*(GameWorld&, GameEntity*, PropertyValueMap const&)>;
extern std::unordered_map<mage::String, EntityFactoryCallback> gEntityFactoryFunctions;

class EntityFactoryFunction
{
public:
	EntityFactoryFunction(mage::StringView inName, EntityFactoryCallback&& inFunction)
	{
		gEntityFactoryFunctions[inName] = inFunction;
	}
};

#define REGISTER_ENTITY_FACTORY_FUNCTION(Type) \
EntityFactoryFunction Type##FactoryFunction(#Type, [](GameWorld& inWorld, GameEntity* inParentEntity, PropertyValueMap const& inProperties) \
{ \
	Type::Setup setup; \
	PropertyTree(setup).ApplyPropertyValues(inProperties); \
	return inWorld.CreateEntity<Type>(inParentEntity, setup); \
});
