#pragma once

#include "Core/NonCopyable.h"

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

class Window : public NonMovableClass
{
public:
	Window(int32_t width, int32_t height, const std::string& name);
	~Window();

	static void PollEvents();

	bool ShouldClose() const;

	VkExtent2D GetExtent() const;

	bool WasWindowResized() const;
	void ResetWindowResizedFlag();

	void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	int GetKeyState(int key) const { return glfwGetKey(mWindow, key); }

	glm::vec2 ConsumeMovement() { glm::vec2 value = mUnconsumedCursorMovement; mUnconsumedCursorMovement = glm::vec2(0.0f); return value; }

	bool TEMP_mPendingFire = false;

private:
	int32_t mWidth;
	int32_t mHeight;

	double mCursorPosX;
	double mCursorPosY;

	glm::vec2 mUnconsumedCursorMovement = glm::vec2(0.0f);

	bool mFramebufferResized = false;

	std::string mName;

	GLFWwindow* mWindow;

	static uint32_t sWindowCount;

	static void IncrementWindowCount();
	static void DecrementWindowCount();

	static void FramebufferResizedCallback(GLFWwindow* glfwWindow, int32_t width, int32_t height);
	static void KeyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);
	static void CursorPositionCallback(GLFWwindow* glfwWindow, double posX, double posY);
};
