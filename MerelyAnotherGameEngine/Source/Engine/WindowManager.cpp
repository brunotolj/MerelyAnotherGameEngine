#include "Engine/WindowManager.h"
#include "Engine/Engine.h"

#include <GLFW/glfw3.h>

void WindowHandle::DestroyWindow()
{
	gEngine->mWindowManager.DestroyWindow(*this);
}

glm::i32vec2 WindowHandle::GetWindowSize() const
{
	return gEngine->mWindowManager.GetWindowSize(*this);
}

CursorInputMode WindowHandle::GetCursorInputMode() const
{
	return gEngine->mWindowManager.GetCursorInputMode(*this);
}

void WindowHandle::SetCursorInputMode(CursorInputMode inValue) const
{
	gEngine->mWindowManager.SetCursorInputMode(*this, inValue);
}

bool WindowHandle::IsKeyPressed(i32 inKey) const
{
	return gEngine->mWindowManager.IsKeyPressed(*this, inKey);
}

glm::dvec2 WindowHandle::GetCursorPosition() const
{
	return gEngine->mWindowManager.GetCursorPosition(*this);
}

WindowManager::WindowManager()
{
	glfwInit();
}

WindowManager::~WindowManager()
{
	for (GLFWwindow* window : mWindows)
		glfwDestroyWindow(window);

	glfwTerminate();
}

u32 WindowManager::GetWindowCount() const
{
	return mWindows.GetSize();
}

WindowHandle WindowManager::CreateWindow(WindowInfo const& inWindowInfo)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(inWindowInfo.Width, inWindowInfo.Height, inWindowInfo.Name.GetCString(), nullptr, nullptr);
	mage_check(window);

	mWindows.Add(window);
	mFocusedWindow = window;

	SetCursorInputMode(window, inWindowInfo.CursorMode);

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, FramebufferResizedCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, CursorPositionCallback);
	glfwSetWindowFocusCallback(window, FocusCallback);

	return window;
}

void WindowManager::DestroyWindow(WindowHandle inWindow)
{
	if (mWindows.Remove(inWindow.mGlfwWindow))
		glfwDestroyWindow(inWindow.mGlfwWindow);
}

glm::i32vec2 WindowManager::GetWindowSize(WindowHandle inWindow) const
{
	if (!mWindows.Contains(inWindow.mGlfwWindow))
		return glm::i32vec2(0, 0);

	glm::i32vec2 result;
	glfwGetFramebufferSize(inWindow.mGlfwWindow, &result.x, &result.y);

	return result;
}

CursorInputMode WindowManager::GetCursorInputMode(WindowHandle inWindow) const
{
	if (!mWindows.Contains(inWindow.mGlfwWindow))
		return CursorInputMode::Normal;

	return CursorInputMode(glfwGetInputMode(inWindow.mGlfwWindow, GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
}

void WindowManager::SetCursorInputMode(WindowHandle inWindow, CursorInputMode inValue) const
{
	if (mWindows.Contains(inWindow.mGlfwWindow))
		glfwSetInputMode(inWindow.mGlfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL + i32(inValue));
}

bool WindowManager::IsKeyPressed(WindowHandle inWindow, i32 inKey) const
{
	if (!mWindows.Contains(inWindow.mGlfwWindow))
		return false;

	return glfwGetKey(inWindow.mGlfwWindow, inKey) == GLFW_PRESS;
}

glm::dvec2 WindowManager::GetCursorPosition(WindowHandle inWindow) const
{
	if (!mWindows.Contains(inWindow.mGlfwWindow))
		return glm::dvec2(0.0, 0.0);

	glm::dvec2 pos;
	glfwGetCursorPos(inWindow.mGlfwWindow, &pos.x, &pos.y);

	return pos;
}

void WindowManager::PollEvents() const
{
	glfwPollEvents();
}

void WindowManager::AddResizedCallback(ResizedCallbackType&& inCallback)
{
	mResizedCallbacks.Add(std::move(inCallback));
}

void WindowManager::AddKeyCallback(KeyCallbackType&& inCallback)
{
	mKeyCallbacks.Add(std::move(inCallback));
}

void WindowManager::AddCursorPositionCallback(CursorPositionCallbackType&& inCallback)
{
	mCursorPositionCallbacks.Add(std::move(inCallback));
}

void WindowManager::FramebufferResizedCallback(GLFWwindow* inGlfwWindow, i32 inNewWidth, i32 inNewHeight)
{
	WindowManager* manager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(inGlfwWindow));

	for (ResizedCallbackType const& callback : manager->mResizedCallbacks)
		callback(inGlfwWindow, glm::i32vec2(inNewWidth, inNewHeight));
}

void WindowManager::KeyCallback(GLFWwindow* inGlfwWindow, i32 inKey, i32 inScancode, i32 inAction, i32 inMods)
{
	WindowManager* manager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(inGlfwWindow));

	for (KeyCallbackType const& callback : manager->mKeyCallbacks)
		callback(inGlfwWindow, inKey, inScancode, inAction, inMods);
}

void WindowManager::CursorPositionCallback(GLFWwindow* inGlfwWindow, f64 inPosX, f64 inPosY)
{
	WindowManager* manager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(inGlfwWindow));

	for (CursorPositionCallbackType const& callback : manager->mCursorPositionCallbacks)
		callback(inGlfwWindow, glm::dvec2(inPosX, inPosY));
}

void WindowManager::FocusCallback(GLFWwindow* inGlfwWindow, i32 inFocused)
{
	WindowManager* manager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(inGlfwWindow));

	if (inFocused)
	{
		manager->mFocusedWindow = inGlfwWindow;
	}
	else if (manager->mFocusedWindow == inGlfwWindow)
	{
		manager->mFocusedWindow = nullptr;
	}
}
