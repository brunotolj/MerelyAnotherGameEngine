#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace slang
{
	struct IGlobalSession;
	struct ISession;
}

namespace Vulkan
{
	struct PipelineShaderStageInfo
	{
		vk::ShaderStageFlagBits Stage;
		mage::StringView EntryPoint;
	};

	struct PipelineCreateInfo
	{
		mage::Array<u32> ShaderCode;
		mage::Array<PipelineShaderStageInfo> ShaderStages;
		mage::Array<vk::VertexInputBindingDescription> InputBindingDescriptions;
		mage::Array<vk::VertexInputAttributeDescription> InputAttributeDescriptions;
		mage::Array<vk::DescriptorSetLayoutBinding> DescriptorSetBindings;
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
}
