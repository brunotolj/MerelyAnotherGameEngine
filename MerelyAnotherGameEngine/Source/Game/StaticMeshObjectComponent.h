#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"

#include <memory>

#include <glm/glm.hpp>

class Model;

template<>
struct ComponentTemplate<class StaticMeshObjectComponent>
{
	std::shared_ptr<Model> Model = nullptr;

	glm::vec3 Color = glm::vec3(1.0f);
};

class StaticMeshObjectComponent : public GameObjectComponent<TransformableObject>
{
public:
	StaticMeshObjectComponent(TransformableObject& owner, const ComponentTemplate<StaticMeshObjectComponent>& creationTemplate);

	std::shared_ptr<Model> const& GetModel() const { return mModel; }

	mage::Transform const& GetTransform() const { return mOwner.Transform; }

	glm::vec3 GetColor() const { return mColor; }

private:
	std::shared_ptr<Model> mModel;

	glm::vec3 mColor;
};
