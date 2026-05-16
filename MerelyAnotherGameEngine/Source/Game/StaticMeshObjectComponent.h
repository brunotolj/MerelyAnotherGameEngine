#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"
#include "Assets/StaticMesh.h"
#include "Assets/Texture.h"

template<>
struct ComponentTemplate<class StaticMeshObjectComponent>
{
	AssetHandle<StaticMesh> Mesh;
	AssetHandle<Texture> Texture;
};

class StaticMeshObjectComponent : public GameObjectComponent<TransformableObject>
{
public:
	StaticMeshObjectComponent(TransformableObject& owner, const ComponentTemplate<StaticMeshObjectComponent>& creationTemplate);

	mage::Transform const& GetTransform() const { return mOwner.Transform; }
	AssetHandle<StaticMesh> GetMesh() const { return mMesh; }
	AssetHandle<Texture> GetTexture() const { return mTexture; }

private:
	AssetHandle<StaticMesh> mMesh;
	AssetHandle<Texture> mTexture;
};
