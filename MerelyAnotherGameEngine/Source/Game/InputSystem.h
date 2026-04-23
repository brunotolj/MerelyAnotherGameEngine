#pragma once

#include <map>

namespace Vulkan
{
	class Window;
}

class InputSystem : public NonCopyableClass
{
public:
	InputSystem(Vulkan::Window& window);

	~InputSystem() {}

	i32 GetKeyState(i32 key);

	void BindKeyInputHandler(i32 key, i32 action, std::function<void()> handler) { mKeyInputHandlers[std::make_pair(key, action)] = handler; }

	void BindCursorMovementHandler(std::function<void(glm::dvec2, i32)> handler) { mCursorMovementHandler = handler; }

private:
	Vulkan::Window& mWindow;

	glm::dvec2 mCursorPosition;

	std::map<std::pair<i32, i32>, std::function<void()>> mKeyInputHandlers;
	std::function<void(glm::dvec2, i32)> mCursorMovementHandler;

	void KeyCallback(i32 key, i32 action, i32 mods);
	void CursorPositionCallback(glm::dvec2 position);
};
