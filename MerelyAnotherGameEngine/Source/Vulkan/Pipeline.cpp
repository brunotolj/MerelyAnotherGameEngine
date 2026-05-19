#include "Vulkan/Pipeline.h"

#include <iostream>
#include <slang/slang.h>

namespace Vulkan
{
	Pipeline& Pipeline::operator=(Pipeline&& inPipeline)
	{
		mDescriptorSetLayout = std::move(inPipeline.mDescriptorSetLayout);
		mPipelineLayout = std::move(inPipeline.mPipelineLayout);
		mVkPipeline = std::move(inPipeline.mVkPipeline);

		return *this;
	}

	void Pipeline::Bind(vk::CommandBuffer inCommandBuffer) const
    {
		inCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mVkPipeline);
    }

	void Pipeline::PushConstants(vk::CommandBuffer inCommandBuffer, vk::PushConstantsInfo inPushInfo) const
	{
		inPushInfo.layout = mPipelineLayout;
		inCommandBuffer.pushConstants2(inPushInfo);
	}

	void Pipeline::PushDescriptorSet(vk::CommandBuffer inCommandBuffer, vk::PushDescriptorSetInfo inPushInfo) const
	{
		inPushInfo.layout = mPipelineLayout;
		inCommandBuffer.pushDescriptorSet2(inPushInfo);
	}
}
