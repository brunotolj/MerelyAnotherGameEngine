#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

namespace Vulkan
{
	struct WindowInfo
	{
		mage::StringView Name;
		i32 Width;
		i32 Height;
	};

	class Window : public NonMovableClass
	{
		friend class Instance;

	public:
		using ResizedCallbackType = std::function<void(i32, i32)>;
		using KeyCallbackType = std::function<void(i32, i32, i32, i32)>;
		using CursorPositionCallbackType = std::function<void(glm::dvec2)>;

		~Window();

		bool ShouldClose() const { return glfwWindowShouldClose(mGlfwWindow); }
		void RequestClose() { glfwSetWindowShouldClose(mGlfwWindow, true); }

		vk::Extent2D GetSize() const { return { u32(mWidth), u32(mHeight) }; }

		i32 GetCursorInputMode() const { return glfwGetInputMode(mGlfwWindow, GLFW_CURSOR); }
		void SetCursorInputMode(i32 inValue) { glfwSetInputMode(mGlfwWindow, GLFW_CURSOR, inValue); }

		i32 GetKeyState(i32 inKey) const { return glfwGetKey(mGlfwWindow, inKey); }
		glm::dvec2 GetCursorPosition() const { glm::dvec2 pos; glfwGetCursorPos(mGlfwWindow, &pos.x, &pos.y); return pos; }

		void SetResizedCallback(ResizedCallbackType&& inCallback) { mResizedCallback = inCallback; }
		void SetKeyCallback(KeyCallbackType&& inCallback) { mKeyCallback = inCallback; }
		void SetCursorPositionCallback(CursorPositionCallbackType&& inCallback) { mCursorPositionCallback = inCallback; }

		void WaitForValidSize() const;

		static void PollEvents() { glfwPollEvents(); }

	private:
		Window(const WindowInfo& inCreateInfo);

		mage::StringView mName;

		i32 mWidth = 0;
		i32 mHeight = 0;

		GLFWwindow* mGlfwWindow = nullptr;

		ResizedCallbackType mResizedCallback;
		KeyCallbackType mKeyCallback;
		CursorPositionCallbackType mCursorPositionCallback;

		static void FramebufferResizedCallback(GLFWwindow* inGlfwWindow, i32 inNewWidth, i32 inNewHeight);
		static void KeyCallback(GLFWwindow* inGlfwWindow, i32 inKey, i32 inScancode, i32 inAction, i32 inMods);
		static void CursorPositionCallback(GLFWwindow* inGlfwWindow, f64 inPosX, f64 inPosY);
	};
}
