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
