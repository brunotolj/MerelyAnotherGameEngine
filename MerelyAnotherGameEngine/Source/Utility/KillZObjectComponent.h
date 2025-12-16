#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"

template<>
struct ComponentTemplate<class KillZObjectComponent>
{
	f32 KillZ = 0.0f;
};

class KillZObjectComponent : public GameObjectComponent<TransformableObject>
{
public:
	KillZObjectComponent(TransformableObject& owner, const ComponentTemplate<KillZObjectComponent>& creationTemplate);

protected:
	virtual void UpdatePostPhysics(f32 deltaTime) override final;

private:
	f32 mKillZ;
};
