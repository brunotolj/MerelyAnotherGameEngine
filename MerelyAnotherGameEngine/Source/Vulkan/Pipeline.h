#pragma once

#include <glslang/Include/glslang_c_shader_types.h>
#include <slang/slang-com-ptr.h>
#include <vulkan/vulkan_raii.hpp>

namespace slang
{
	struct IGlobalSession;
	struct ISession;
}

namespace Vulkan
{
	using SpirVBinary = mage::Array<u32>;

	struct PipelineCreateInfo
	{
		SpirVBinary ShaderCode;
		mage::Array<vk::VertexInputBindingDescription> BindingDescriptions;
		mage::Array<vk::VertexInputAttributeDescription> AttributeDescriptions;
		vk::PipelineInputAssemblyStateCreateInfo InputAssemblyInfo;
		mage::Array<vk::DescriptorSetLayoutBinding> DescriptorSetLayout;
		mage::Array<vk::PushConstantRange> PushConstantRanges;
	};

	class Pipeline : public NonCopyableClass
	{
		friend class Renderer;

	public:
		Pipeline(Pipeline&& inPipeline) { *this = std::move(inPipeline); };
		Pipeline& operator=(Pipeline&& inPipeline);

		void Bind(vk::CommandBuffer inCommandBuffer) const;
		void PushConstants(vk::CommandBuffer inCommandBuffer, vk::PushConstantsInfo inPushInfo) const;
		void PushDescriptorSet(vk::CommandBuffer inCommandBuffer, vk::PushDescriptorSetInfo inPushInfo) const;

	private:
		Pipeline() {}

		vk::raii::DescriptorSetLayout mDescriptorSetLayout = nullptr;
		vk::raii::PipelineLayout mPipelineLayout = nullptr;
		vk::raii::Pipeline mVkPipeline = nullptr;
	};

	class ShaderCompiler : public NonMovableClass
	{
	public:
		ShaderCompiler();

		SpirVBinary CompileFromFile(mage::StringView inPath) const;

	private:
		Slang::ComPtr<slang::IGlobalSession> mGlobalSession = nullptr;
		Slang::ComPtr<slang::ISession> mSession = nullptr;
	};

}
