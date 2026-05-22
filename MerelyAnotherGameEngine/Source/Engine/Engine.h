#pragma once

#include "Engine/AssetManager.h"
#include "Engine/ShaderCompiler.h"
#include "Vulkan/Device.h"

class Engine : public NonMovableClass
{
public:
	Vulkan::Device mVulkanDevice;

	AssetManager mAssetManager;

	ShaderCompiler mShaderCompiler;
};

extern Engine* gEngine;
