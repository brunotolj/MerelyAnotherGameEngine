#pragma once

#include "Property.h"

class GameEntity;
class GameWorld;
struct GameEntitySetup;

class GameWorldComponent : public NonMovable
{
public:
	static bool CheckPrerequisites(GameWorld& inWorld) { return true; }

	GameWorldComponent(GameWorld& inWorld) : mWorld(inWorld) {}
	virtual ~GameWorldComponent() {}

	virtual void GetEntityCallbackTypes(mage::Array<std::type_index>& outTypes) const {}

	virtual void OnEntityCreated(GameEntity* inEntity, GameEntitySetup const* inSetup, std::type_index inType) {}
	virtual void OnEntityDestroyed(GameEntity* inEntity, std::type_index inType) {}

protected:
	GameWorld& mWorld;
};

template <typename Type>
concept WorldComponentType = std::derived_from<Type, GameWorldComponent> && !std::same_as<Type, GameWorldComponent>;

class GameUtility : public GameWorldComponent
{
public:
	GameUtility(GameWorld& inWorld) : GameWorldComponent(inWorld) {}

	virtual void PreSystemsUpdate() {};
	virtual void PostSystemsUpdate() {};
};

class GameSystem : public GameWorldComponent
{
public:
	GameSystem(GameWorld& inWorld) : GameWorldComponent(inWorld) {}

	virtual void Update(f32 inDeltaTime) {};
};

template <WorldComponentType Type>
struct Property<Type> : public PropertyNode
{
	static void GetChildProperties(PropertyTree& inOutContainer) {}
};

using PropertyValueMap = std::unordered_map<mage::String, mage::String>;
using WorldComponentFactoryCallback = std::function<void(GameWorld&, PropertyValueMap const&)>;

extern std::unordered_map<mage::String, WorldComponentFactoryCallback> gWorldComponentFactoryFunctions;

class WorldComponentFactoryFunction
{
public:
	WorldComponentFactoryFunction(mage::StringView inName, WorldComponentFactoryCallback&& inFunction)
	{
		gWorldComponentFactoryFunctions[inName] = inFunction;
	}
};

#define REGISTER_WORLD_COMPONENT_FACTORY_FUNCTION(Type) \
WorldComponentFactoryFunction Type##FactoryFunction(#Type, [](GameWorld& inWorld, PropertyValueMap const& inProperties) \
{ \
	inWorld.CreateComponent<Type>(inProperties); \
});
