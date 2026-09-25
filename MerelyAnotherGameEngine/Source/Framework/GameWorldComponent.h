#pragma once

class GameWorld;

class GameWorldComponent : public NonMovable
{
public:
	static bool CheckPrerequisites(GameWorld& inWorld) { return true; }

	GameWorldComponent(GameWorld& inWorld) : mWorld(inWorld) {}
	virtual ~GameWorldComponent() {}

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

using PropertyContainerOld = std::unordered_map<mage::String, mage::String>;
using WorldComponentFactoryCallback = std::function<void(GameWorld&, PropertyContainerOld const&)>;

extern std::unordered_map<mage::String, WorldComponentFactoryCallback> gWorldComponentFactoryFunctions;

class WorldComponentFactoryFunction
{
public:
	WorldComponentFactoryFunction(mage::StringView inName, WorldComponentFactoryCallback&& inFunction)
	{
		gWorldComponentFactoryFunctions[inName] = inFunction;
	}
};
