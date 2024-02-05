#pragma once

#include "Game/GameObjectComponent.h"
#include "Rendering/RenderCommon.h"

#include <memory>

#include <glm/glm.hpp>

class Model;
class TransformableObject;

class StaticMeshObjectComponent : public GameObjectComponent<TransformableObject>, public Renderable
{
public:
	std::shared_ptr<Model> mModel;

	glm::vec3 mColor = glm::vec3(1.0f);

	StaticMeshObjectComponent(TransformableObject& owner);

	virtual mage::Transform GetTransform() const override;

	virtual glm::vec3 GetColor() const override;

	virtual void Bind(VkCommandBuffer_T* commandBuffer) const override;

	virtual void Draw(VkCommandBuffer_T* commandBuffer) const override;

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) override;

	virtual void OnOwnerRemovedFromWorld(GameWorld& world) override;
};
