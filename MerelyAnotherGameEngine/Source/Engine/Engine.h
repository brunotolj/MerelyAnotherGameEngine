#pragma once

#include "Engine/AssetManager.h"
#include "Engine/InputHandler.h"
#include "Engine/PhysicsEngine_PhysX.h"
#include "Engine/ShaderCompiler.h"
#include "Engine/WindowManager.h"
#include "Vulkan/Device.h"

class Engine : public NonMovable
{
public:
	Engine();

	bool ShouldExit() const;
	void RequestExit();

private:
	class GlobalPtrSetter
	{
	public:
		GlobalPtrSetter(Engine* inEngine);
		~GlobalPtrSetter();
	} mGlobalPtrSetter;

public:
	WindowManager mWindowManager;

	Vulkan::Device mVulkanDevice;

	ShaderCompiler mShaderCompiler;

	PhysicsEngine_PhysX mPhysicsEngine;

	InputHandler mInputHandler;

	AssetManager mAssetManager;

private:
	bool mExitRequested = false;
};

extern Engine* gEngine;
