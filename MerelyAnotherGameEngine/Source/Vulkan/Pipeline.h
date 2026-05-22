#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace slang
{
	struct IGlobalSession;
	struct ISession;
}

namespace Vulkan
{
	class Pipeline : public NonMovableClass
	{
	public:
		struct ShaderStageInfo
		{
			vk::ShaderStageFlagBits Stage;
			mage::StringView EntryPoint;
		};

		struct CreateInfo
		{
			mage::Array<u32> ShaderCode;
			mage::Array<ShaderStageInfo> ShaderStages;
			mage::Array<vk::VertexInputBindingDescription> InputBindingDescriptions;
			mage::Array<vk::VertexInputAttributeDescription> InputAttributeDescriptions;
			mage::Array<vk::DescriptorSetLayoutBinding> DescriptorSetBindings;
			mage::Array<vk::PushConstantRange> PushConstantRanges;
		};

		Pipeline(nullptr_t) {};
		Pipeline(CreateInfo const& inCreateInfo);

		void Create(CreateInfo const& inCreateInfo);

		void Bind(vk::CommandBuffer inCommandBuffer) const;
		void PushConstants(vk::CommandBuffer inCommandBuffer, vk::PushConstantsInfo inPushInfo) const;
		void PushDescriptorSet(vk::CommandBuffer inCommandBuffer, vk::PushDescriptorSetInfo inPushInfo) const;

	private:
		vk::raii::DescriptorSetLayout mDescriptorSetLayout = nullptr;
		vk::raii::PipelineLayout mPipelineLayout = nullptr;
		vk::raii::Pipeline mVkPipeline = nullptr;
	};
}
