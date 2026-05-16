#pragma once

#include "Game/GameObject.h"
#include "Game/GameObjectComponent.h"
#include "Assets/StaticMesh.h"
#include "Assets/Texture.h"

template<>
struct ComponentTemplate<class BallSpawnerComponent>
{
	PhysicsRigidBodyParams RigidBodyParams;
	AssetHandle<StaticMesh> Mesh;
	AssetHandle<Texture> Texture;
	f32 Speed = 10.0f;
	i32 InputSpawn = 70; // #FixMe: GLFW_KEY_F
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
	AssetHandle<StaticMesh> mMesh;
	AssetHandle<Texture> mTexture;
	f32 mSpeed;
	i32 mInputSpawn;

	bool mPendingBallSpawn = false;

private:
	void SpawnBall();
};
