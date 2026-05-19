#pragma once

#include "Engine/AssetManager.h"
#include "Engine/ShaderCompiler.h"

class Engine : public NonMovableClass
{
public:
	AssetManager mAssetManager;

	ShaderCompiler mShaderCompiler;
};

extern Engine* gEngine;
