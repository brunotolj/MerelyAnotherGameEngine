#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"

template<>
struct ComponentTemplate<class KillZObjectComponent>
{
	float KillZ = 0.0f;
};

class KillZObjectComponent : public GameObjectComponent<TransformableObject>
{
public:
	KillZObjectComponent(TransformableObject& owner, const ComponentTemplate<KillZObjectComponent>& creationTemplate);

protected:
	virtual void UpdatePostPhysics(float deltaTime) override final;

private:
	float mKillZ;
};
