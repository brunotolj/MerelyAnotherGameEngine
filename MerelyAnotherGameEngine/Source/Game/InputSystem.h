#pragma once

#include "Core/NonCopyable.h"

#include <functional>
#include <map>

#include <glm/glm.hpp>

class Window;

class InputSystem : public NonCopyableClass
{
public:
	InputSystem(Window& window);

	~InputSystem() {}

	int GetKeyState(int key);

	void BindKeyInputHandler(int key, int action, std::function<void()> handler) { mKeyInputHandlers[std::make_pair(key, action)] = handler; }

	void BindCursorMovementHandler(std::function<void(glm::dvec2, int)> handler) { mCursorMovementHandler = handler; }

private:
	Window& mWindow;

	glm::dvec2 mCursorPosition;

	std::map<std::pair<int, int>, std::function<void()>> mKeyInputHandlers;
	std::function<void(glm::dvec2, int)> mCursorMovementHandler;

	void KeyCallback(int key, int action, int mods);
	void CursorPositionCallback(glm::dvec2 position);
};
