#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"
#include "Rendering/RenderCommon.h"

template<>
struct ComponentTemplate<class CameraComponent>
{
	float NearPlane;
	float FarPlane;
	float HorizontalFOV;
};

class CameraComponent : public GameObjectComponent<TransformableObject>, public ICamera
{
public:
	CameraComponent(TransformableObject& owner, const ComponentTemplate<CameraComponent>& creationTemplate);

	virtual glm::mat4 GetViewTransform() const override final;

	virtual const glm::mat4& GetProjectionTransform() const override final;

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) override final;

	virtual void OnOwnerRemovedFromWorld(GameWorld& world) override final;

	virtual void UpdatePostPhysics(float deltaTime) override final;

private:
	float mNearPlane;
	float mFarPlane;
	float mHorizontalFOV;

	float mAspectRatio;

	glm::mat4 mPerspectiveTransform = glm::mat4(1.0f);

private:
	void CachePerspectiveTransform();
};
