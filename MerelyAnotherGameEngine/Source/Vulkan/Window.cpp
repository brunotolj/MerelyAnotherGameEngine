#include "Vulkan/Window.h"

namespace Vulkan
{
	Window::Window(const WindowInfo& inCreateInfo) :
		mName(inCreateInfo.Name)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		mGlfwWindow = glfwCreateWindow(inCreateInfo.Width, inCreateInfo.Height, mName.GetCString(), nullptr, nullptr);
		mage_check(mGlfwWindow);

		glfwGetFramebufferSize(mGlfwWindow, &mWidth, &mHeight);

		glfwSetWindowUserPointer(mGlfwWindow, this);
		glfwSetInputMode(mGlfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		glfwSetFramebufferSizeCallback(mGlfwWindow, FramebufferResizedCallback);
		glfwSetKeyCallback(mGlfwWindow, KeyCallback);
		glfwSetCursorPosCallback(mGlfwWindow, CursorPositionCallback);
	}

	Window::~Window()
	{
		glfwDestroyWindow(mGlfwWindow);
	}

	void Window::WaitForValidSize() const
	{
		while (mWidth == 0 || mHeight == 0)
			glfwWaitEvents();

		return;
	}

	void Window::FramebufferResizedCallback(GLFWwindow* inGlfwWindow, i32 inNewWidth, i32 inNewHeight)
	{
		Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(inGlfwWindow));
		window->mWidth = inNewWidth;
		window->mHeight = inNewHeight;

		if (window->mResizedCallback != nullptr)
			window->mResizedCallback(inNewWidth, inNewHeight);
	}

	void Window::KeyCallback(GLFWwindow* inGlfwWindow, i32 inKey, i32 inScancode, i32 inAction, i32 inMods)
	{
		Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(inGlfwWindow));

		if (window->mKeyCallback != nullptr)
			window->mKeyCallback(inKey, inScancode, inAction, inMods);
	}

	void Window::CursorPositionCallback(GLFWwindow* inGlfwWindow, f64 inPosX, f64 inPosY)
	{
		Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(inGlfwWindow));

		if (window->mCursorPositionCallback != nullptr)
			window->mCursorPositionCallback(glm::dvec2(inPosX, inPosY));
	}
}
