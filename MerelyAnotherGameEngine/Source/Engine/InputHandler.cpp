#include "Engine/InputHandler.h"
#include "Engine/Engine.h"

InputHandler::InputHandler()
{
	WindowManager& windowManager = gEngine->mWindowManager;

	windowManager.AddKeyCallback([this](WindowHandle inWindow, i32 inKey, i32 inScancode, i32 inAction, i32 inMods) { KeyCallback(inKey, inAction, inMods); });
	windowManager.AddCursorPositionCallback([this](WindowHandle inWindow, glm::dvec2 inPosition) { CursorPositionCallback(inPosition); });

	mCursorPosition = windowManager.GetFocusedWindow().GetCursorPosition();
}

bool InputHandler::IsKeyPressed(i32 key)
{
	WindowHandle window = gEngine->mWindowManager.GetFocusedWindow();
	return window.IsKeyPressed(key);
}

void InputHandler::KeyCallback(i32 key, i32 action, i32 mods)
{
	std::function<void()> const& handler = mKeyInputHandlers[std::make_pair(key, action)];

	if (handler != nullptr)
	{
		handler();
	}
}

void InputHandler::CursorPositionCallback(glm::dvec2 position)
{
	glm::dvec2 movement = position - mCursorPosition;

	if (mCursorMovementHandler != nullptr)
	{
		WindowHandle window = gEngine->mWindowManager.GetFocusedWindow();
		mCursorMovementHandler(movement, window.GetCursorInputMode());
	}

	mCursorPosition = position;
}
