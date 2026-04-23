#include "Game/StaticMeshObjectComponent.h"

StaticMeshObjectComponent::StaticMeshObjectComponent(TransformableObject& owner, const ComponentTemplate<StaticMeshObjectComponent>& creationTemplate) :
	GameObjectComponent(owner), mModel(creationTemplate.Model), mColor(creationTemplate.Color)
{
}
