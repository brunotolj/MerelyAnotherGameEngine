#pragma once

enum class CursorInputMode : u8
{
	Normal,
	Hidden,
	Disabled
};

struct WindowInfo
{
	mage::StringView Name;
	i32 Width = 0;
	i32 Height = 0;
	CursorInputMode CursorMode = CursorInputMode::Normal;
};

struct GLFWwindow;

class WindowHandle
{
	friend class WindowManager;

public:
	void DestroyWindow();

	glm::i32vec2 GetWindowSize() const;

	CursorInputMode GetCursorInputMode() const;
	void SetCursorInputMode(CursorInputMode inValue) const;

	bool IsKeyPressed(i32 inKey) const;
	glm::dvec2 GetCursorPosition() const;

	GLFWwindow* GetRawWindow() const { return mGlfwWindow; }

	bool operator==(WindowHandle const& inOther) const = default;
	bool operator!=(WindowHandle const& inOther) const = default;

private:
	WindowHandle(GLFWwindow* inGlfwWindow) : mGlfwWindow(inGlfwWindow) {}

	GLFWwindow* mGlfwWindow = nullptr;
};

class WindowManager : public NonMovable
{
public:
	WindowManager();
	~WindowManager();

	u32 GetWindowCount() const;
	WindowHandle GetFocusedWindow() const { return mFocusedWindow; }

	WindowHandle CreateWindow(WindowInfo const& inWindowInfo);
	void DestroyWindow(WindowHandle inWindow);

	glm::i32vec2 GetWindowSize(WindowHandle inWindow) const;

	CursorInputMode GetCursorInputMode(WindowHandle inWindow) const;
	void SetCursorInputMode(WindowHandle inWindow, CursorInputMode inValue) const;

	bool IsKeyPressed(WindowHandle inWindow, i32 inKey) const;
	glm::dvec2 GetCursorPosition(WindowHandle inWindow) const;

	void PollEvents() const;

	using ResizedCallbackType = std::function<void(WindowHandle, glm::i32vec2)>;
	void AddResizedCallback(ResizedCallbackType&& inCallback);

	using KeyCallbackType = std::function<void(WindowHandle, i32, i32, i32, i32)>;
	void AddKeyCallback(KeyCallbackType&& inCallback);

	using CursorPositionCallbackType = std::function<void(WindowHandle, glm::dvec2)>;
	void AddCursorPositionCallback(CursorPositionCallbackType&& inCallback);

private:
	mage::Array<GLFWwindow*> mWindows;
	GLFWwindow* mFocusedWindow = nullptr;

	mage::Array<ResizedCallbackType> mResizedCallbacks;
	mage::Array<KeyCallbackType> mKeyCallbacks;
	mage::Array<CursorPositionCallbackType> mCursorPositionCallbacks;

	static void FramebufferResizedCallback(GLFWwindow* inGlfwWindow, i32 inNewWidth, i32 inNewHeight);
	static void KeyCallback(GLFWwindow* inGlfwWindow, i32 inKey, i32 inScancode, i32 inAction, i32 inMods);
	static void CursorPositionCallback(GLFWwindow* inGlfwWindow, f64 inPosX, f64 inPosY);
	static void FocusCallback(GLFWwindow* inGlfwWindow, i32 inFocused);
};
