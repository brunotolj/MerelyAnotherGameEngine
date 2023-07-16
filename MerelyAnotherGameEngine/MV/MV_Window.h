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
		int32_t privWidth;
		int32_t privHeight;

		bool privFramebufferResized = false;

		std::string privName;

		GLFWwindow* privWindow;

		static uint32_t sWindowCount;

		static void IncrementWindowCount();
		static void DecrementWindowCount();

		static void FramebufferResizedCallback(GLFWwindow* glfwWindow, int32_t width, int32_t height);
	};
}
