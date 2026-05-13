#pragma once

#include <memory>
#include <vector>

class GameObject;
class PhysicsSystem;
class InputSystem;
class MeshRenderSystem;
class SpriteRenderSystem;
class TextRenderSystem;

namespace Vulkan
{
	class Renderer;
}

class GameWorld : public NonCopyableClass
{
public:
	GameWorld(
		std::unique_ptr<InputSystem>&& inputSystem,
		std::unique_ptr<PhysicsSystem>&& physicsSystem,
		std::unique_ptr<MeshRenderSystem>&& meshRenderSystem,
		std::unique_ptr<SpriteRenderSystem>&& spriteRenderSystem,
		std::unique_ptr<TextRenderSystem>&& textRenderSystem);

	void Update(f32 deltaTime);
	void Render(Vulkan::Renderer& renderer) const;

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
	std::unique_ptr<TextRenderSystem> mTextRenderSystem;

	std::vector<std::shared_ptr<GameObject>> mObjects;
	std::vector<std::shared_ptr<GameObject>> mNewObjects;

	bool mIsCurrentlyUpdatingObjects = false;
};
