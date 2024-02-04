#include "Rendering/StaticMeshObjectComponent.h"
#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Rendering/RenderSystem.h"

StaticMeshObjectComponent::StaticMeshObjectComponent(TransformableObject& owner) :
	GameObjectComponent(owner)
{
}

glm::mat4 StaticMeshObjectComponent::GetTransformMatrix() const
{
	return mOwner.Transform.Matrix();
}

void StaticMeshObjectComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	world.GetRenderSystem().AddStaticMesh(this);
}

void StaticMeshObjectComponent::OnOwnerRemovedFromWorld(GameWorld& world)
{
	world.GetRenderSystem().RemoveStaticMesh(this);
}
