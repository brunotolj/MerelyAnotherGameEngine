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

glm::mat4 StaticMeshObjectComponent::GetNormalMatrix() const
{
	const mage::Transform& transform = mOwner.mTransform; 

	const float s = transform.Rotation.S;
	const float p = transform.Rotation.XY;
	const float q = transform.Rotation.YZ;
	const float r = transform.Rotation.ZX;
	float a = s + p; a *= a;
	float b = s + q; b *= b;
	float c = s + r; c *= c;
	float d = s - p; d *= d;
	float e = s - q; e *= e;
	float f = s - r; f *= f;
	float g = p + q; g *= g;
	float h = q + r; h *= h;
	float i = r + p; i *= i;

	return
	{
		glm::vec4{ (b + e) - 1.0f, (h + d) - 1.0f, (g + c) - 1.0f, 0.0f } / transform.Scale.x,
		glm::vec4{ (h + a) - 1.0f, (c + f) - 1.0f, (i + e) - 1.0f, 0.0f } / transform.Scale.y,
		glm::vec4{ (g + f) - 1.0f, (i + b) - 1.0f, (a + d) - 1.0f, 0.0f } / transform.Scale.z,
		glm::vec4(0.0f),
	};
}

void StaticMeshObjectComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	world.mRenderSystem->AddStaticMesh(this);
}

void StaticMeshObjectComponent::OnOwnerRemovedFromWorld(GameWorld& world)
{
	world.mRenderSystem->RemoveStaticMesh(this);
}
