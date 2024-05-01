#include "Game/InputSystem.h"
#include "Rendering/Window.h"

InputSystem::InputSystem(Window& window) : mWindow(window)
{
	mWindow.SetKeyCallback([this](int key, int scancode, int action, int mods) { KeyCallback(key, action, mods); });
	mWindow.SetCursorPositionCallback([this](glm::dvec2 position) { CursorPositionCallback(position); });

	mCursorPosition = mWindow.GetCursorPosition();
}

int InputSystem::GetKeyState(int key) { return mWindow.GetKeyState(key); }

void InputSystem::KeyCallback(int key, int action, int mods)
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
