#include "Engine/ShaderCompiler.h"

#include <iostream>
#include <slang/slang.h>

ShaderCompiler::ShaderCompiler()
{
	slang::createGlobalSession(mGlobalSession.writeRef());

	slang::TargetDesc targetDesc
	{
		.format = SLANG_SPIRV,
		.profile = mGlobalSession->findProfile("spirv_1_6"),
		.flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY | SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM
	};

	slang::SessionDesc sessionDesc
	{
		.targets = &targetDesc,
		.targetCount = 1,
		.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR
	};

	mGlobalSession->createSession(sessionDesc, mSession.writeRef());
}

SpirVBinary ShaderCompiler::CompileFromFile(mage::StringView inPath) const
{
	slang::IModule* slangModule = nullptr;
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		slangModule = mSession->loadModule(inPath.GetCString(), diagnosticsBlob.writeRef());

		if (diagnosticsBlob != nullptr)
			std::cout << cstr(diagnosticsBlob->getBufferPointer());

		if (slangModule == nullptr)
			return SpirVBinary();
	}

	mage::Array<slang::IComponentType*> componentTypes;
	componentTypes.Add(slangModule);

	Slang::ComPtr<slang::IComponentType> composedProgram;
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		SlangResult result = mSession->createCompositeComponentType(
			componentTypes.GetData(),
			componentTypes.GetSize(),
			composedProgram.writeRef(),
			diagnosticsBlob.writeRef());

		if (diagnosticsBlob != nullptr)
			std::cout << cstr(diagnosticsBlob->getBufferPointer());

		if (SLANG_FAILED(result))
			return SpirVBinary();
	}

	Slang::ComPtr<slang::IBlob> spirvCode;
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		SlangResult result = composedProgram->getTargetCode(0, spirvCode.writeRef(), diagnosticsBlob.writeRef());

		if (diagnosticsBlob != nullptr)
			std::cout << cstr(diagnosticsBlob->getBufferPointer());

		if (SLANG_FAILED(result))
			return SpirVBinary();
	}

	SpirVBinary binary;
	binary.ResizeUninitialized(u32(spirvCode->getBufferSize()) / sizeof(u32));
	memcpy(binary.GetData(), spirvCode->getBufferPointer(), sizeof(u32) * binary.GetSize());

	return binary;
}
