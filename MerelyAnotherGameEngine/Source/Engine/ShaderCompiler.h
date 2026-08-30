#pragma once

#include <slang/slang-com-ptr.h>

namespace slang
{
	struct IGlobalSession;
	struct ISession;
}

using SpirVBinary = mage::Array<u32>;

class ShaderCompiler : public NonMovable
{
public:
	ShaderCompiler();

	SpirVBinary CompileFromFile(mage::StringView inPath) const;

private:
	Slang::ComPtr<slang::IGlobalSession> mGlobalSession = nullptr;
	Slang::ComPtr<slang::ISession> mSession = nullptr;
};
