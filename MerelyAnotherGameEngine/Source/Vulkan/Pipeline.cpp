#include "Vulkan/Pipeline.h"
#include "Engine/Engine.h"

namespace Vulkan
{
    Pipeline::Pipeline(CreateInfo const& inCreateInfo)
    {
		Create(inCreateInfo);
    }

	void Pipeline::Create(CreateInfo const& inCreateInfo)
	{
		vk::raii::Device const& device = gEngine->mVulkanDevice.GetVkDevice();

		mage_check(inCreateInfo.ShaderCode.GetSize());

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
		{
			.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptor,
			.bindingCount = inCreateInfo.DescriptorSetBindings.GetSize(),
			.pBindings = inCreateInfo.DescriptorSetBindings.GetData()
		};

		mDescriptorSetLayout = device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo
		{
			.setLayoutCount = 1,
			.pSetLayouts = &*mDescriptorSetLayout,
			.pushConstantRangeCount = inCreateInfo.PushConstantRanges.GetSize(),
			.pPushConstantRanges = inCreateInfo.PushConstantRanges.GetData()
		};

		mPipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

		vk::ShaderModuleCreateInfo shaderModuleCreateInfo
		{
			.codeSize = inCreateInfo.ShaderCode.GetSize() * sizeof(u32),
			.pCode = inCreateInfo.ShaderCode.GetData()
		};

		vk::raii::ShaderModule shaderModule{ device, shaderModuleCreateInfo };

		mage::Array<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos;

		for (ShaderStageInfo const& shaderStageInfo : inCreateInfo.ShaderStages)
			shaderStageCreateInfos.Add
			({
				.stage = shaderStageInfo.Stage,
				.module = shaderModule,
				.pName = shaderStageInfo.EntryPoint.GetCString()
				});

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo
		{
			.vertexBindingDescriptionCount = inCreateInfo.InputBindingDescriptions.GetSize(),
			.pVertexBindingDescriptions = inCreateInfo.InputBindingDescriptions.GetData(),
			.vertexAttributeDescriptionCount = inCreateInfo.InputAttributeDescriptions.GetSize(),
			.pVertexAttributeDescriptions = inCreateInfo.InputAttributeDescriptions.GetData()
		};

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};

		vk::PipelineViewportStateCreateInfo viewportInfo{};

		vk::PipelineRasterizationStateCreateInfo rasterizationInfo
		{
			.frontFace = vk::FrontFace::eClockwise,
			.lineWidth = 1.0f
		};

		vk::PipelineMultisampleStateCreateInfo multisampleInfo
		{
			.sampleShadingEnable = vk::True,
			.minSampleShading = 0.2f
		};

		vk::PipelineColorBlendAttachmentState colorBlendAttachmentState
		{
			.blendEnable = vk::True,
			.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
			.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.colorBlendOp = vk::BlendOp::eAdd,
			.srcAlphaBlendFactor = vk::BlendFactor::eOne,
			.dstAlphaBlendFactor = vk::BlendFactor::eZero,
			.alphaBlendOp = vk::BlendOp::eAdd,
			.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
		};

		vk::PipelineDepthStencilStateCreateInfo depthStencilInfo
		{
			.depthCompareOp = vk::CompareOp::eLess
		};

		vk::PipelineColorBlendStateCreateInfo colorBlendInfo
		{
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachmentState
		};

		mage::Array<vk::DynamicState> dynamicStates
		{
			vk::DynamicState::eCullMode,
			vk::DynamicState::eFrontFace,
			vk::DynamicState::ePrimitiveTopology,
			vk::DynamicState::eViewportWithCount,
			vk::DynamicState::eScissorWithCount,
			vk::DynamicState::eDepthTestEnable,
			vk::DynamicState::eDepthWriteEnable,
			vk::DynamicState::ePolygonModeEXT,
			vk::DynamicState::eRasterizationSamplesEXT,
			vk::DynamicState::eConservativeRasterizationModeEXT,
			vk::DynamicState::eExtraPrimitiveOverestimationSizeEXT
		};

		vk::PipelineDynamicStateCreateInfo dynamicStateInfo
		{
			.dynamicStateCount = dynamicStates.GetSize(),
			.pDynamicStates = dynamicStates.GetData()
		};

		vk::Format format = vk::Format::eB8G8R8A8Srgb;

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain
		{
			{
				.stageCount = shaderStageCreateInfos.GetSize(),
				.pStages = shaderStageCreateInfos.GetData(),
				.pVertexInputState = &vertexInputInfo,
				.pInputAssemblyState = &inputAssemblyInfo,
				.pViewportState = &viewportInfo,
				.pRasterizationState = &rasterizationInfo,
				.pMultisampleState = &multisampleInfo,
				.pDepthStencilState = &depthStencilInfo,
				.pColorBlendState = &colorBlendInfo,
				.pDynamicState = &dynamicStateInfo,
				.layout = mPipelineLayout,
				.renderPass = nullptr
			},
			{
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &format,
				.depthAttachmentFormat = vk::Format::eD32Sfloat
			}
		};

		mVkPipeline = device.createGraphicsPipeline(nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
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
