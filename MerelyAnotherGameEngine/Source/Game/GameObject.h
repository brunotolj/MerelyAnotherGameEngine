#pragma once

#include "Core/NonCopyable.h"
#include "Core/Transform.h"

#include <memory>
#include <vector>

class GameObjectComponent;
class GameWorld;

class GameObject : public NonCopyableClass
{
	friend GameWorld;

public:
	mage::Transform mTransform; // #TODO: not every object should have a transform

	template<typename ComponentClass, typename... Args>
	ComponentClass* const CreateComponent()
	{
		static_assert(std::is_base_of<GameObjectComponent, ComponentClass>::value, "ComponentClass must be derived from GameObjectComponent");

		std::unique_ptr<ComponentClass> component = std::make_unique<ComponentClass>(*this, Args...);
		ComponentClass* const componentPtr = component.get();
		mComponents.push_back(std::move(component));

		return componentPtr;
	}

protected:
	void OnAddedToWorld(GameWorld& world);

	void OnRemovedFromWorld(GameWorld& world);
	
	void UpdatePrePhysics(float deltaTime);

	void UpdatePostPhysics(float deltaTime);

private:
	GameWorld* mWorld;

	std::vector<std::unique_ptr<GameObjectComponent>> mComponents;
};
