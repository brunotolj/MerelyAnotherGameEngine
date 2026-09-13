#include "Framework/GameWorld.h"
#include "Engine/Engine.h"
#include "Game/RigidBodyObjectComponent.h"
#include "Game/StaticMeshObjectComponent.h"
#include "Physics/PhysicsCommon.h"
#include "Utility/BallSpawnerComponent.h"

BallSpawnerComponent::BallSpawnerComponent(TransformableObject& owner, const ComponentTemplate<BallSpawnerComponent>& creationTemplate) :
	GameObjectComponent(owner),
	mRigidBodyParams(creationTemplate.RigidBodyParams),
	mMesh(creationTemplate.Mesh),
	mTexture(creationTemplate.Texture),
	mSpeed(creationTemplate.Speed),
	mInputSpawn(creationTemplate.InputSpawn)
{
}

void BallSpawnerComponent::OnOwnerAddedToWorld()
{
	gEngine->mInputHandler.BindKeyInputHandler(mInputSpawn, 1, [this]() { mPendingBallSpawn = true; }); // #TODO: GLFW_PRESS
}

void BallSpawnerComponent::Update(f32 deltaTime)
{
	if (mPendingBallSpawn)
	{
		SpawnBall();
		mPendingBallSpawn = false;
	}
}

void BallSpawnerComponent::SpawnBall()
{
	const glm::vec3 forward = mOwner.GetTransform().Rotation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f));

	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams = mRigidBodyParams;
	rigidBodyTemplate.InitialLinearVelocity = mSpeed * reinterpret_cast<const physx::PxVec3&>(forward);

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Mesh = mMesh;
	staticMeshTemplate.Texture = mTexture;

	mOwner.mWorld.CreateObject<TransformableObject>(mOwner.GetTransform(), rigidBodyTemplate, staticMeshTemplate);
}
