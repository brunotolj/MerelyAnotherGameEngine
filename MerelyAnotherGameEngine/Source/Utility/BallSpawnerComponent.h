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
	float Speed = 10.0f;
	int InputSpawn = GLFW_KEY_F;
};

class BallSpawnerComponent : public GameObjectComponent<TransformableObject>
{
public:
	BallSpawnerComponent(TransformableObject& owner, const ComponentTemplate<BallSpawnerComponent>& creationTemplate);

protected:
	virtual void OnOwnerAddedToWorld(GameWorld& world) override final;

	virtual void UpdatePrePhysics(float deltaTime) override final;

private:
	PhysicsRigidBodyParams mRigidBodyParams;
	std::shared_ptr<Model> mModel;
	glm::vec3 mColor;
	float mSpeed;
	int mInputSpawn;

	bool mPendingBallSpawn = false;

private:
	void SpawnBall();
};
