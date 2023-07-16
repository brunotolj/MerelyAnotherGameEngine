#pragma once

#include "NonCopyable.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace MV
{
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

	private:
		int32_t mWidth;
		int32_t mHeight;
		bool mFramebufferResized = false;

		std::string mName;

		GLFWwindow* mWindow;

		static uint32_t sWindowCount;

	private:
		static void IncrementWindowCount();
		static void DecrementWindowCount();

		static void FramebufferResizedCallback(GLFWwindow* glfwWindow, int32_t width, int32_t height);
	};
}
