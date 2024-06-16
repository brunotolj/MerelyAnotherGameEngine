#include "Game/GameWorld.h"
#include "Utility/KillZObjectComponent.h"

KillZObjectComponent::KillZObjectComponent(TransformableObject& owner, const ComponentTemplate<KillZObjectComponent>& creationTemplate) :
	GameObjectComponent(owner),
	mKillZ(creationTemplate.KillZ)
{
}

void KillZObjectComponent::UpdatePostPhysics(float deltaTime)
{
	if (mOwner.Transform.Position.z < mKillZ)
	{
		mOwner.Destroy();
	}
}
