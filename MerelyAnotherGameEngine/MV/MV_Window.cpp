#include "MV/MV_Window.h"
#include "Asserts.h"

using MV::Window;

Window::Window(int width, int height, const std::string& name) :
	privWidth(width), privHeight(height), privName(name)
{
	IncrementWindowCount();
	privWindow = glfwCreateWindow(privWidth, privHeight, privName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(privWindow, this);
	glfwSetFramebufferSizeCallback(privWindow, FramebufferResizedCallback);
}

Window::~Window()
{
	glfwDestroyWindow(privWindow);
	DecrementWindowCount();
}

uint32_t Window::sWindowCount = 0;

void Window::PollEvents()
{
	glfwPollEvents();
}

bool MV::Window::ShouldClose() const
{
	return glfwWindowShouldClose(privWindow);
}

VkExtent2D MV::Window::GetExtent() const
{
	return { static_cast<uint32_t>(privWidth), static_cast<uint32_t>(privHeight) };
}

bool MV::Window::WasWindowResized() const
{
	return privFramebufferResized;
}

void MV::Window::ResetWindowResizedFlag()
{
	privFramebufferResized = false;
}

void MV::Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
	check(glfwCreateWindowSurface(instance, privWindow, nullptr, surface) == VK_SUCCESS);
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
	window->privFramebufferResized = true;
	window->privWidth = width;
	window->privHeight = height;
}
