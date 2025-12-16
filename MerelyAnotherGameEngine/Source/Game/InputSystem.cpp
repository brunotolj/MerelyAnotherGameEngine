#include "Game/InputSystem.h"
#include "Rendering/Window.h"

InputSystem::InputSystem(Window& window) : mWindow(window)
{
	mWindow.SetKeyCallback([this](i32 key, i32 scancode, i32 action, i32 mods) { KeyCallback(key, action, mods); });
	mWindow.SetCursorPositionCallback([this](glm::dvec2 position) { CursorPositionCallback(position); });

	mCursorPosition = mWindow.GetCursorPosition();
}

i32 InputSystem::GetKeyState(i32 key) { return mWindow.GetKeyState(key); }

void InputSystem::KeyCallback(i32 key, i32 action, i32 mods)
{
	const std::function<void()>& handler = mKeyInputHandlers[std::make_pair(key, action)];
	if (handler != nullptr) handler();
}

void InputSystem::CursorPositionCallback(glm::dvec2 position)
{
	const glm::dvec2 movement = position - mCursorPosition;
	if (mCursorMovementHandler != nullptr) mCursorMovementHandler(movement, mWindow.GetCursorInputMode());

	mCursorPosition = position;
}
