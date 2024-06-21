#include "Game/GameWorld.h"
#include "Game/InputSystem.h"
#include "Game/RigidBodyObjectComponent.h"
#include "Game/StaticMeshObjectComponent.h"
#include "Physics/PhysicsCommon.h"
#include "Utility/BallSpawnerComponent.h"
#include "Utility/KillZObjectComponent.h"

BallSpawnerComponent::BallSpawnerComponent(TransformableObject& owner, const ComponentTemplate<BallSpawnerComponent>& creationTemplate) :
	GameObjectComponent(owner),
	mRigidBodyParams(creationTemplate.RigidBodyParams),
	mModel(creationTemplate.Model),
	mColor(creationTemplate.Color),
	mSpeed(creationTemplate.Speed),
	mInputSpawn(creationTemplate.InputSpawn)
{
}

void BallSpawnerComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	world.GetInputSystem().BindKeyInputHandler(mInputSpawn, GLFW_PRESS, [this]() { mPendingBallSpawn = true; });
}

void BallSpawnerComponent::UpdatePrePhysics(float deltaTime)
{
	if (mPendingBallSpawn)
	{
		SpawnBall();
		mPendingBallSpawn = false;
	}
}

void BallSpawnerComponent::SpawnBall()
{
	const glm::vec3 forward = mOwner.Transform.Rotation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f));

	std::shared_ptr<TransformableObject> ballPtr = std::make_shared<TransformableObject>();
	TransformableObject& ball = *ballPtr.get();
	ball.Transform = mOwner.Transform;

	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams = mRigidBodyParams;
	rigidBodyTemplate.InitialLinearVelocity = mSpeed * reinterpret_cast<const physx::PxVec3&>(forward);
	GameObject::CreateComponent(ball, rigidBodyTemplate);

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Model = mModel;
	staticMeshTemplate.Color = mColor;
	GameObject::CreateComponent(ball, staticMeshTemplate);

	ComponentTemplate<KillZObjectComponent> killZTemplate;
	killZTemplate.KillZ = -10.0f;
	GameObject::CreateComponent(ball, killZTemplate);

	mOwner.GetWorld()->AddObject(ballPtr);
}
