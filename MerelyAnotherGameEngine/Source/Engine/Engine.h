#pragma once

#include "Engine/AssetManager.h"
#include "Engine/ShaderCompiler.h"
#include "Engine/WindowManager.h"
#include "Vulkan/Device.h"

class Engine : public NonMovableClass
{
public:
	bool ShouldExit() const;
	void RequestExit();

	WindowManager mWindowManager;

	Vulkan::Device mVulkanDevice;

	AssetManager mAssetManager;

	ShaderCompiler mShaderCompiler;

private:
	bool mExitRequested = false;
};

extern Engine* gEngine;
