#include "MV/MV_Window.h"
#include "Asserts.h"

using MV::Window;

Window::Window(int width, int height, const std::string& name) :
	mWidth(width), mHeight(height), mName(name)
{
	IncrementWindowCount();
	mWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, FramebufferResizedCallback);
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
	check(glfwCreateWindowSurface(instance, mWindow, nullptr, surface) == VK_SUCCESS);
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
	check(sWindowCount > 0);
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
