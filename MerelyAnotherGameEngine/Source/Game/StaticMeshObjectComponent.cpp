#include "Game/StaticMeshObjectComponent.h"
#include "Game/GameWorld.h"
#include "Rendering/Model.h"
#include "Rendering/Systems/MeshRenderSystem.h"

StaticMeshObjectComponent::StaticMeshObjectComponent(TransformableObject& owner, const ComponentTemplate<StaticMeshObjectComponent>& creationTemplate) :
	GameObjectComponent(owner), mModel(creationTemplate.Model), mColor(creationTemplate.Color)
{
}
