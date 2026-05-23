#pragma once

#include <map>

enum class CursorInputMode : u8;

class InputSystem : public NonCopyableClass
{
public:
	InputSystem();

	~InputSystem() {}

	bool IsKeyPressed(i32 key);

	void BindKeyInputHandler(i32 key, i32 action, std::function<void()> handler) { mKeyInputHandlers[std::make_pair(key, action)] = handler; }
	void BindCursorMovementHandler(std::function<void(glm::dvec2, CursorInputMode)> handler) { mCursorMovementHandler = handler; }

private:
	glm::dvec2 mCursorPosition;

	std::map<std::pair<i32, i32>, std::function<void()>> mKeyInputHandlers;
	std::function<void(glm::dvec2, CursorInputMode)> mCursorMovementHandler;

	void KeyCallback(i32 key, i32 action, i32 mods);
	void CursorPositionCallback(glm::dvec2 position);
};
