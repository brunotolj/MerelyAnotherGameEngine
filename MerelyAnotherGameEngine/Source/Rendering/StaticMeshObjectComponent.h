#pragma once

#include "Game/GameObjectComponent.h"

#include <memory>

#include <glm/glm.hpp>

class Model;
class TransformableObject;

class StaticMeshObjectComponent : public GameObjectComponent<TransformableObject>
{
public:
	std::shared_ptr<Model> mModel;

	glm::vec3 mColor = glm::vec3(1.0f);

	StaticMeshObjectComponent(TransformableObject& owner);

	glm::mat4 GetTransformMatrix() const;

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) override;

	virtual void OnOwnerRemovedFromWorld(GameWorld& world) override;
};
