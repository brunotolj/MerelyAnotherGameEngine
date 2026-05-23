#pragma once

#include "Game/InputSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Systems/MeshRenderSystem.h"
#include "Rendering/Systems/SpriteRenderSystem.h"
#include "Rendering/Systems/TextRenderSystem.h"

#include <memory>
#include <vector>

class GameObject;

namespace Vulkan
{
	class Renderer;
}

class GameWorld : public NonCopyableClass
{
public:
	void Update(f32 deltaTime);
	void Render(Vulkan::Renderer& renderer) const;

	void AddObject(const std::shared_ptr<GameObject>& object);
	void RemoveObject(const std::shared_ptr<GameObject>& object);

	InputSystem mInputSystem;
	PhysicsSystem mPhysicsSystem;
	MeshRenderSystem mMeshRenderSystem;
	SpriteRenderSystem mSpriteRenderSystem;
	TextRenderSystem mTextRenderSystem;

private:
	std::vector<std::shared_ptr<GameObject>> mObjects;
	std::vector<std::shared_ptr<GameObject>> mNewObjects;

	bool mIsCurrentlyUpdatingObjects = false;
};
