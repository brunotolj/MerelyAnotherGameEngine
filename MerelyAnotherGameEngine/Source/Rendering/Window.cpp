#include "Core/Asserts.h"
#include "Rendering/Window.h"

Window::Window(int width, int height, const std::string& name) :
	mWidth(width), mHeight(height), mName(name)
{
	IncrementWindowCount();
	mWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(mWindow, this);

	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetFramebufferSizeCallback(mWindow, FramebufferResizedCallback);
	glfwSetKeyCallback(mWindow, KeyCallback);
	glfwSetCursorPosCallback(mWindow, CursorPositionCallback);
}

Window::~Window()
{
	glfwDestroyWindow(mWindow);
	DecrementWindowCount();
}

uint32_t Window::sWindowCount = 0;

VkExtent2D Window::GetExtent() const
{
	return { static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight) };
}

void Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
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

void Window::FramebufferResizedCallback(GLFWwindow* glfwWindow, int32_t width, int32_t height)
{
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
	window->mFramebufferResized = true;
	window->mWidth = width;
	window->mHeight = height;
}

void Window::KeyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods)
{
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
	if (window->mKeyCallback != nullptr) window->mKeyCallback(key, scancode, action, mods);
}

void Window::CursorPositionCallback(GLFWwindow* glfwWindow, double posX, double posY)
{
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
	if (window->mCursorPositionCallback != nullptr) window->mCursorPositionCallback(glm::dvec2(posX, posY));
}
