#include "Core/Asserts.h"
#include "Rendering/MV_Window.h"

using MV::Window;

Window::Window(int width, int height, const std::string& name) :
	mWidth(width), mHeight(height), mName(name)
{
	IncrementWindowCount();
	mWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, FramebufferResizedCallback);
	glfwSetKeyCallback(mWindow, KeyCallback);

	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(mWindow, &mCursorPosX, &mCursorPosY);
	glfwSetCursorPosCallback(mWindow, CursorPositionCallback);
}

Window::~Window()
{
	glfwDestroyWindow(mWindow);
	DecrementWindowCount();
}

uint32_t Window::sWindowCount = 0;

void Window::PollEvents()
{
	glfwPollEvents();
}

bool MV::Window::ShouldClose() const
{
	return glfwWindowShouldClose(mWindow);
}

VkExtent2D MV::Window::GetExtent() const
{
	return { static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight) };
}

bool MV::Window::WasWindowResized() const
{
	return mFramebufferResized;
}

void MV::Window::ResetWindowResizedFlag()
{
	mFramebufferResized = false;
}

void MV::Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
	mage_check(glfwCreateWindowSurface(instance, mWindow, nullptr, surface) == VK_SUCCESS);
}

void Window::IncrementWindowCount()
{
	if (sWindowCount == 0)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	}

	sWindowCount++;
}

void Window::DecrementWindowCount()
{
	mage_check(sWindowCount > 0);
	sWindowCount--;

	if (sWindowCount == 0)
	{
		glfwTerminate();
	}
}

void MV::Window::FramebufferResizedCallback(GLFWwindow* glfwWindow, int32_t width, int32_t height)
{
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
	window->mFramebufferResized = true;
	window->mWidth = width;
	window->mHeight = height;
}

void MV::Window::KeyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(glfwWindow, true);
		return;
	}

	if (key == GLFW_KEY_LEFT_CONTROL)
	{
		if (action == GLFW_PRESS && glfwGetInputMode(glfwWindow, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		{
			glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			return;
		}

		if (action == GLFW_RELEASE && glfwGetInputMode(glfwWindow, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
		{
			Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
			glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwGetCursorPos(glfwWindow, &window->mCursorPosX, &window->mCursorPosY);
			return;
		}
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		window->TEMP_mPendingFire = true;
	}
}

void MV::Window::CursorPositionCallback(GLFWwindow* glfwWindow, double posX, double posY)
{
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

	if (glfwGetInputMode(glfwWindow, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
		window->mUnconsumedCursorMovement += glm::vec2(posX - window->mCursorPosX, posY - window->mCursorPosY);
	}

	window->mCursorPosX = posX;
	window->mCursorPosY = posY;
}
