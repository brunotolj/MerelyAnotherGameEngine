#include "Game/StaticMeshObjectComponent.h"
#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Rendering/Model.h"
#include "Rendering/RenderSystem.h"

StaticMeshObjectComponent::StaticMeshObjectComponent(TransformableObject& owner) :
	GameObjectComponent(owner)
{
}

mage::Transform StaticMeshObjectComponent::GetTransform() const
{
	return mOwner.Transform;
}

glm::vec3 StaticMeshObjectComponent::GetColor() const
{
	return mColor;
}

void StaticMeshObjectComponent::Bind(VkCommandBuffer_T* commandBuffer) const
{
	mModel->Bind(commandBuffer);
}

void StaticMeshObjectComponent::Draw(VkCommandBuffer_T* commandBuffer) const
{
	mModel->Draw(commandBuffer);
}

void StaticMeshObjectComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	world.GetRenderSystem().AddRenderable(this);
}

void StaticMeshObjectComponent::OnOwnerRemovedFromWorld(GameWorld& world)
{
	world.GetRenderSystem().RemoveRenderable(this);
}
