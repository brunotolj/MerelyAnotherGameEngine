#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"

template<>
struct ComponentTemplate<class CameraComponent>
{
};

class CameraComponent : public GameObjectComponent<TransformableObject>
{
public:
	CameraComponent(TransformableObject& owner, const ComponentTemplate<CameraComponent>& creationTemplate);

	glm::mat4 GetViewTransform() const;
};
