#include "Rendering/StaticMeshObjectComponent.h"
#include "Game/GameObject.h"
#include "Game/GameWorld.h"
#include "Rendering/RenderSystem.h"

StaticMeshObjectComponent::StaticMeshObjectComponent(GameObject& owner) :
	GameObjectComponent(owner)
{
}

glm::mat4 StaticMeshObjectComponent::GetTransformMatrix() const
{
	return mOwner.mTransform.Matrix();
}

void StaticMeshObjectComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	world.mRenderSystem->AddStaticMesh(this);
}

void StaticMeshObjectComponent::OnOwnerRemovedFromWorld(GameWorld& world)
{
	world.mRenderSystem->RemoveStaticMesh(this);
}
