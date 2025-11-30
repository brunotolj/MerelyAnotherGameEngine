#include "Core/Asserts.h"
#include "Game/CameraComponent.h"
#include "Game/GameWorld.h"
#include "Rendering/Systems/MeshRenderSystem.h"

CameraComponent::CameraComponent(TransformableObject& owner, const ComponentTemplate<CameraComponent>& creationTemplate) :
	GameObjectComponent(owner)
{
}

glm::mat4 CameraComponent::GetViewTransform() const
{
	return glm::inverse(mOwner.Transform.Matrix());
}
