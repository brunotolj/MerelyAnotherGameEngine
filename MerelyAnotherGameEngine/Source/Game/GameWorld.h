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
	std::unique_ptr<RenderSystem> mRenderSystem;

	std::unique_ptr<PhysicsSystem> mPhysicsSystem;

	void Update(float deltaTime);

	void AddObject(const std::shared_ptr<GameObject>& object);

	void RemoveObject(const std::shared_ptr<GameObject>& object);

//private:
	std::vector<std::shared_ptr<GameObject>> mObjects;
};
