#include "Engine/Engine.h"

Engine* gEngine = nullptr;

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
