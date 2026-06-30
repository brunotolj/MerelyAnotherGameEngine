#include "Engine/Engine.h"

Engine* gEngine = nullptr;

Engine::Engine() : mGlobalPtrSetter(this)
{
}

bool Engine::ShouldExit() const
{
	if (mWindowManager.GetWindowCount() == 0)
		return true;

	return mExitRequested;
}

void Engine::RequestExit()
{
	mExitRequested = true;
}

Engine::GlobalPtrSetter::GlobalPtrSetter(Engine* inEngine)
{
	mage_check(gEngine == nullptr);
	gEngine = inEngine;
}

Engine::GlobalPtrSetter::~GlobalPtrSetter()
{
	gEngine = nullptr;
}
