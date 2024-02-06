#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"
#include "Rendering/RenderCommon.h"

#include <memory>

#include <glm/glm.hpp>

class Model;

template<>
struct ComponentTemplate<class StaticMeshObjectComponent>
{
	std::shared_ptr<Model> Model = nullptr;

	glm::vec3 Color = glm::vec3(1.0f);
};

class StaticMeshObjectComponent : public GameObjectComponent<TransformableObject>, public Renderable
{
public:
	StaticMeshObjectComponent(TransformableObject& owner, const ComponentTemplate<StaticMeshObjectComponent>& creationTemplate);

	virtual mage::Transform GetTransform() const override final;

	virtual glm::vec3 GetColor() const override final;

	virtual void Bind(VkCommandBuffer_T* commandBuffer) const override final;

	virtual void Draw(VkCommandBuffer_T* commandBuffer) const override final;

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) override final;

	virtual void OnOwnerRemovedFromWorld(GameWorld& world) override final;

private:
	std::shared_ptr<Model> mModel;

	glm::vec3 mColor;
};
