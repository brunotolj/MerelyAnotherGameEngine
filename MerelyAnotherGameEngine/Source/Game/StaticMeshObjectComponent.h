#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"

namespace Vulkan
{
	class Model;
}

template<>
struct ComponentTemplate<class StaticMeshObjectComponent>
{
	std::shared_ptr<Vulkan::Model> Model = nullptr;
	u32 TextureIndex = 0;
};

class StaticMeshObjectComponent : public GameObjectComponent<TransformableObject>
{
public:
	StaticMeshObjectComponent(TransformableObject& owner, const ComponentTemplate<StaticMeshObjectComponent>& creationTemplate);

	std::shared_ptr<Vulkan::Model> GetModel() const { return mModel; }

	mage::Transform const& GetTransform() const { return mOwner.Transform; }

	u32 GetTextureIndex() const { return mTextureIndex; }

private:
	std::shared_ptr<Vulkan::Model> mModel;
	u32 mTextureIndex;
};
