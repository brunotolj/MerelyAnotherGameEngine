#pragma once

#include "Physics/PhysicsSystem.h"
#include "Rendering/Systems/MeshRenderSystem.h"
#include "Rendering/Systems/SpriteRenderSystem.h"
#include "Rendering/Systems/TextRenderSystem.h"

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

	template <typename ObjectClass, typename... Args>
	ObjectClass* CreateObject(Args&&... inArgs)
	{
		ObjectClass* object = new ObjectClass(inArgs...);

		if (mIsCurrentlyUpdatingObjects)
			mNewObjects.Add(object);
		else
			mObjects.Add(object);

		object->OnAddedToWorld(*this);
		return object;
	}

	PhysicsSystem mPhysicsSystem;
	MeshRenderSystem mMeshRenderSystem;
	SpriteRenderSystem mSpriteRenderSystem;
	TextRenderSystem mTextRenderSystem;

private:
	mage::Array<GameObject*> mObjects;
	mage::Array<GameObject*> mNewObjects;

	bool mIsCurrentlyUpdatingObjects = false;
};
