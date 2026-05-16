#include "Game/StaticMeshObjectComponent.h"

StaticMeshObjectComponent::StaticMeshObjectComponent(TransformableObject& owner, const ComponentTemplate<StaticMeshObjectComponent>& creationTemplate) :
	GameObjectComponent(owner), mMesh(creationTemplate.Mesh), mTexture(creationTemplate.Texture)
{
}
