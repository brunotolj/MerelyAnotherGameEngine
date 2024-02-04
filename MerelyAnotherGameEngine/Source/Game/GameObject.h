#pragma once

#include "Core/NonCopyable.h"
#include "Core/Transform.h"

#include <memory>
#include <vector>

class GameObjectComponentBase;
class GameWorld;

class GameObject : public NonCopyableClass
{
	friend GameWorld;

public:
	template<typename ComponentClass, typename... Args>
	ComponentClass* const CreateComponent()
	{
		static_assert(std::is_base_of<GameObjectComponentBase, ComponentClass>::value, "ComponentClass must be derived from GameObjectComponent");

		auto& ownerRef = reinterpret_cast<decltype(std::declval<ComponentClass>().mOwner)&>(*this);
		std::unique_ptr<ComponentClass> component = std::make_unique<ComponentClass>(ownerRef, Args...);
		ComponentClass* const componentPtr = component.get();
		mComponents.push_back(std::move(component));

		return componentPtr;
	}

	bool IsDestroyed() const { return mIsDestoryed; }
	void MarkDestroyed() { mIsDestoryed = true; }

protected:
	void OnAddedToWorld(GameWorld& world);

	void OnRemovedFromWorld(GameWorld& world);
	
	void UpdatePrePhysics(float deltaTime);

	void UpdatePostPhysics(float deltaTime);

private:
	GameWorld* mWorld = nullptr;

	std::vector<std::unique_ptr<GameObjectComponentBase>> mComponents;

	bool mIsDestoryed = false;
};

class TransformableObject : public GameObject
{
public:
	mage::Transform Transform;
};
