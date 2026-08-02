#include "Game/CameraComponent.h"

CameraComponent::CameraComponent(TransformableObject& owner, const ComponentTemplate<CameraComponent>& creationTemplate) :
	GameObjectComponent(owner)
{
}

glm::mat4 CameraComponent::GetViewTransform() const
{
	return mOwner.GetTransform().Inverse().Matrix();
}
