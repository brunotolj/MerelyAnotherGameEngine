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

	struct DescriptorSetLayoutInfo
	{
		mage::Array<vk::DescriptorSetLayoutBinding> Bindings;
		u32 Count = 1;
	};

	struct PipelineCreateInfo
	{
		SpirVBinary ShaderCode;
		mage::Array<vk::VertexInputBindingDescription> BindingDescriptions;
		mage::Array<vk::VertexInputAttributeDescription> AttributeDescriptions;
		vk::PipelineInputAssemblyStateCreateInfo InputAssemblyInfo;
		mage::Array<DescriptorSetLayoutInfo> DescriptorSetLayouts;
		mage::Array<vk::DescriptorPoolSize> DescriptorPoolSizes;
		mage::Array<vk::PushConstantRange> PushConstantRanges;
	};

	class Pipeline : public NonCopyableClass
	{
		friend class Renderer;

	public:
		Pipeline(Pipeline&& inPipeline) { *this = std::move(inPipeline); };
		Pipeline& operator=(Pipeline&& inPipeline);

		void Bind(vk::CommandBuffer inCommandBuffer) const;
		void BindDescriptorSet(vk::CommandBuffer inCommandBuffer, vk::BindDescriptorSetsInfo inBindInfo, u32 inDescriptorSetIndex) const;
		void PushConstants(vk::CommandBuffer inCommandBuffer, vk::PushConstantsInfo inPushInfo) const;

		void UpdateDescriptorSet(vk::WriteDescriptorSet inWrite, u32 inDescriptorSetIndex) const;

	private:
		Pipeline() {}

		mage::Array<vk::raii::DescriptorSetLayout> mDescriptorSetLayouts;
		vk::raii::DescriptorPool mDescriptorPool = nullptr;
		mage::Array<vk::raii::DescriptorSet> mDescriptorSets;

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
