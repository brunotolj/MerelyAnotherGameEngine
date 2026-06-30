#include "Game/GameWorld.h"
#include "Engine/Engine.h"
#include "Game/RigidBodyObjectComponent.h"
#include "Game/StaticMeshObjectComponent.h"
#include "Physics/PhysicsCommon.h"
#include "Utility/BallSpawnerComponent.h"
#include "Utility/KillZObjectComponent.h"

BallSpawnerComponent::BallSpawnerComponent(TransformableObject& owner, const ComponentTemplate<BallSpawnerComponent>& creationTemplate) :
	GameObjectComponent(owner),
	mRigidBodyParams(creationTemplate.RigidBodyParams),
	mMesh(creationTemplate.Mesh),
	mTexture(creationTemplate.Texture),
	mSpeed(creationTemplate.Speed),
	mInputSpawn(creationTemplate.InputSpawn)
{
}

void BallSpawnerComponent::OnOwnerAddedToWorld(GameWorld& world)
{
	gEngine->mInputHandler.BindKeyInputHandler(mInputSpawn, 1, [this]() { mPendingBallSpawn = true; }); // #FixMe: GLFW_PRESS
}

void BallSpawnerComponent::UpdatePrePhysics(f32 deltaTime)
{
	if (mPendingBallSpawn)
	{
		SpawnBall();
		mPendingBallSpawn = false;
	}
}

void BallSpawnerComponent::SpawnBall()
{
	const glm::vec3 forward = mOwner.mTransform.Rotation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f));

	ComponentTemplate<RigidBodyObjectComponent> rigidBodyTemplate;
	rigidBodyTemplate.RigidBodyParams = mRigidBodyParams;
	rigidBodyTemplate.InitialLinearVelocity = mSpeed * reinterpret_cast<const physx::PxVec3&>(forward);

	ComponentTemplate<StaticMeshObjectComponent> staticMeshTemplate;
	staticMeshTemplate.Mesh = mMesh;
	staticMeshTemplate.Texture = mTexture;

	ComponentTemplate<KillZObjectComponent> killZTemplate;
	killZTemplate.KillZ = -10.0f;

	mOwner.GetWorld()->CreateObject<TransformableObject>(mOwner.mTransform, rigidBodyTemplate, staticMeshTemplate, killZTemplate);
}
