#pragma once

#include "Core/NonCopyable.h"

#include <functional>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

using KeyCallbackType = std::function<void(int, int, int, int)>;
using CursorPositionCallbackType = std::function<void(glm::dvec2)>;

class Window : public NonMovableClass
{
public:
	Window(int32_t width, int32_t height, const std::string& name);
	~Window();

	static void PollEvents() { glfwPollEvents(); }

	bool ShouldClose() const { return glfwWindowShouldClose(mWindow); }
	void Close() { glfwSetWindowShouldClose(mWindow, true); }

	VkExtent2D GetExtent() const;

	bool WasWindowResized() const { return mFramebufferResized; }
	void ResetWindowResizedFlag() { mFramebufferResized = false; }

	void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	int GetCursorInputMode() const { return glfwGetInputMode(mWindow, GLFW_CURSOR); }
	void SetCursorInputMode(int value) { glfwSetInputMode(mWindow, GLFW_CURSOR, value); }

	int GetKeyState(int key) const { return glfwGetKey(mWindow, key); }
	glm::dvec2 GetCursorPosition() const { glm::dvec2 pos; glfwGetCursorPos(mWindow, &pos.x, &pos.y); return pos; }

	void SetKeyCallback(KeyCallbackType&& value) { mKeyCallback = value; }
	void SetCursorPositionCallback(CursorPositionCallbackType&& value) { mCursorPositionCallback = value; }

private:
	int32_t mWidth;
	int32_t mHeight;

	bool mFramebufferResized = false;

	std::string mName;

	GLFWwindow* mWindow;

	KeyCallbackType mKeyCallback;
	CursorPositionCallbackType mCursorPositionCallback;

	static uint32_t sWindowCount;

	static void IncrementWindowCount();
	static void DecrementWindowCount();

	static void FramebufferResizedCallback(GLFWwindow* glfwWindow, int32_t width, int32_t height);
	static void KeyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);
	static void CursorPositionCallback(GLFWwindow* glfwWindow, double posX, double posY);
};
