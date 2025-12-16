#pragma once

#include <functional>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

using KeyCallbackType = std::function<void(i32, i32, i32, i32)>;
using CursorPositionCallbackType = std::function<void(glm::dvec2)>;

class Window : public NonMovableClass
{
public:
	Window(i32 width, i32 height, const std::string& name);
	~Window();

	static void PollEvents() { glfwPollEvents(); }

	bool ShouldClose() const { return glfwWindowShouldClose(mWindow); }
	void Close() { glfwSetWindowShouldClose(mWindow, true); }

	VkExtent2D GetExtent() const;

	bool WasWindowResized() const { return mFramebufferResized; }
	void ResetWindowResizedFlag() { mFramebufferResized = false; }

	void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	i32 GetCursorInputMode() const { return glfwGetInputMode(mWindow, GLFW_CURSOR); }
	void SetCursorInputMode(i32 value) { glfwSetInputMode(mWindow, GLFW_CURSOR, value); }

	i32 GetKeyState(i32 key) const { return glfwGetKey(mWindow, key); }
	glm::dvec2 GetCursorPosition() const { glm::dvec2 pos; glfwGetCursorPos(mWindow, &pos.x, &pos.y); return pos; }

	void SetKeyCallback(KeyCallbackType&& value) { mKeyCallback = value; }
	void SetCursorPositionCallback(CursorPositionCallbackType&& value) { mCursorPositionCallback = value; }

private:
	i32 mWidth;
	i32 mHeight;

	bool mFramebufferResized = false;

	std::string mName;

	GLFWwindow* mWindow;

	KeyCallbackType mKeyCallback;
	CursorPositionCallbackType mCursorPositionCallback;

	static u32 sWindowCount;

	static void IncrementWindowCount();
	static void DecrementWindowCount();

	static void FramebufferResizedCallback(GLFWwindow* glfwWindow, i32 width, i32 height);
	static void KeyCallback(GLFWwindow* glfwWindow, i32 key, i32 scancode, i32 action, i32 mods);
	static void CursorPositionCallback(GLFWwindow* glfwWindow, f64 posX, f64 posY);
};
