#pragma once

#include "Core/NonCopyable.h"
#include "Core/Transform.h"
#include "Game/GameObjectCommon.h"

#include <map>
#include <memory>
#include <typeindex>
#include <vector>

class GameObjectComponentBase;
class GameWorld;

class GameObject : public NonCopyableClass
{
	friend GameWorld;

public:
	template<GameObjectClass ObjectClass, GameObjectComponentClass ComponentClass>
	static ComponentClass& CreateComponent(ObjectClass& owner, const ComponentTemplate<ComponentClass>& creationTemplate)
	{
		std::unique_ptr<ComponentClass> component = std::make_unique<ComponentClass>(owner, creationTemplate);
		ComponentClass& componentRef = *component.get();
		owner.mComponents.push_back(std::move(component));

		return componentRef;
	}

	template<GameObjectComponentClass ComponentClass>
	std::vector<std::shared_ptr<ComponentClass>> GetComponentsOfClass()
	{
		std::vector<std::shared_ptr<ComponentClass>> result;

		auto componentArray = mComponentsByClass.find(typeid(ComponentClass));

		if (componentArray == mComponentsByClass.end())
			return result;

		for (size_t index : componentArray->second)
			result.push_back(std::reinterpret_pointer_cast<ComponentClass>(mComponents[index]));

		return result;
	}

	bool IsDestroyed() const { return mIsDestoryed; }
	void Destroy() { mIsDestoryed = true; }

	GameWorld* GetWorld() const { return mWorld; }

protected:
	void OnAddedToWorld(GameWorld& world);

	void OnRemovedFromWorld(GameWorld& world);
	
	void UpdatePrePhysics(float deltaTime);

	void UpdatePostPhysics(float deltaTime);

private:
	GameWorld* mWorld = nullptr;

	std::vector<std::shared_ptr<GameObjectComponentBase>> mComponents;

	std::map<std::type_index, std::vector<size_t>> mComponentsByClass;

	bool mIsDestoryed = false;
};

class TransformableObject : public GameObject
{
public:
	mage::Transform Transform;
};
