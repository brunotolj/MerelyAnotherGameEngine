#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"
#include "Rendering/Model.h"

template<>
struct ComponentTemplate<class BallSpawnerComponent>
{
	PhysicsRigidBodyParams RigidBodyParams;
	std::shared_ptr<Model> Model = nullptr;
	glm::vec3 Color = glm::vec3(1.0f);
	f32 Speed = 10.0f;
	i32 InputSpawn = GLFW_KEY_F;
};

class BallSpawnerComponent : public GameObjectComponent<TransformableObject>
{
public:
	BallSpawnerComponent(TransformableObject& owner, const ComponentTemplate<BallSpawnerComponent>& creationTemplate);

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) override final;

	virtual void UpdatePrePhysics(f32 deltaTime) override final;

private:
	PhysicsRigidBodyParams mRigidBodyParams;
	std::shared_ptr<Model> mModel;
	glm::vec3 mColor;
	f32 mSpeed;
	i32 mInputSpawn;

	bool mPendingBallSpawn = false;

private:
	void SpawnBall();
};
