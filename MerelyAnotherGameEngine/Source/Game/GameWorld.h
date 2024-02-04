#pragma once

#include "Core/NonCopyable.h"

#include <memory>
#include <vector>

class GameObject;
class PhysicsSystem;
class RenderSystem;

class GameWorld : public NonCopyableClass
{
public:
	GameWorld(
		std::unique_ptr<RenderSystem>&& renderSystem,
		std::unique_ptr<PhysicsSystem>&& physicsSystem);

	void Update(float deltaTime);

	void AddObject(const std::shared_ptr<GameObject>& object);

	void RemoveObject(const std::shared_ptr<GameObject>& object);

	RenderSystem& GetRenderSystem() const { return *mRenderSystem; }

	PhysicsSystem& GetPhysicsSystem() const { return *mPhysicsSystem; }

private:
	std::vector<std::shared_ptr<GameObject>> mObjects;

	std::unique_ptr<RenderSystem> mRenderSystem;

	std::unique_ptr<PhysicsSystem> mPhysicsSystem;
};
