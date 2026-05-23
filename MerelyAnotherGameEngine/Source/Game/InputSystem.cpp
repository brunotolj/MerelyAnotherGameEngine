#include "Game/InputSystem.h"
#include "Engine/Engine.h"

InputSystem::InputSystem()
{
	WindowManager& windowManager = gEngine->mWindowManager;

	windowManager.AddKeyCallback([this](WindowHandle inWindow, i32 inKey, i32 inScancode, i32 inAction, i32 inMods) { KeyCallback(inKey, inAction, inMods); });
	windowManager.AddCursorPositionCallback([this](WindowHandle inWindow, glm::dvec2 inPosition) { CursorPositionCallback(inPosition); });

	mCursorPosition = windowManager.GetFocusedWindow().GetCursorPosition();
}

bool InputSystem::IsKeyPressed(i32 key)
{
	WindowHandle window = gEngine->mWindowManager.GetFocusedWindow();
	return window.IsKeyPressed(key);
}

void InputSystem::KeyCallback(i32 key, i32 action, i32 mods)
{
	const std::function<void()>& handler = mKeyInputHandlers[std::make_pair(key, action)];

	if (handler != nullptr)
	{
		handler();
	}
}

void InputSystem::CursorPositionCallback(glm::dvec2 position)
{
	const glm::dvec2 movement = position - mCursorPosition;

	if (mCursorMovementHandler != nullptr)
	{
		WindowHandle window = gEngine->mWindowManager.GetFocusedWindow();
		mCursorMovementHandler(movement, window.GetCursorInputMode());
	}

	mCursorPosition = position;
}
