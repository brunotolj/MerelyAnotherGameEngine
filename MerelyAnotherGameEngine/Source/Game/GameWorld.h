#pragma once

#include "Core/NonCopyable.h"

#include <memory>
#include <vector>

class GameObject;
class PhysicsSystem;
class Renderer;
class InputSystem;
class MeshRenderSystem;
class SpriteRenderSystem;

class GameWorld : public NonCopyableClass
{
public:
	GameWorld(
		std::unique_ptr<InputSystem>&& inputSystem,
		std::unique_ptr<PhysicsSystem>&& physicsSystem,
		std::unique_ptr<MeshRenderSystem>&& meshRenderSystem,
		std::unique_ptr<SpriteRenderSystem>&& spriteRenderSystem);

	void Update(float deltaTime);
	void Render(Renderer& renderer) const;

	void AddObject(const std::shared_ptr<GameObject>& object);
	void RemoveObject(const std::shared_ptr<GameObject>& object);

	InputSystem& GetInputSystem() const { return *mInputSystem; }
	PhysicsSystem& GetPhysicsSystem() const { return *mPhysicsSystem; }
	MeshRenderSystem& GetMeshRenderSystem() const { return *mMeshRenderSystem; }
	SpriteRenderSystem& GetSpriteRenderSystem() const { return *mSpriteRenderSystem; }

private:
	std::unique_ptr<InputSystem> mInputSystem;
	std::unique_ptr<PhysicsSystem> mPhysicsSystem;
	std::unique_ptr<MeshRenderSystem> mMeshRenderSystem;
	std::unique_ptr<SpriteRenderSystem> mSpriteRenderSystem;

	std::vector<std::shared_ptr<GameObject>> mObjects;
	std::vector<std::shared_ptr<GameObject>> mNewObjects;

	bool mIsCurrentlyUpdatingObjects = false;
};
