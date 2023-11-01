#include "Core/Asserts.h"
#include "Rendering/MV_Pipeline.h"
#include "Rendering/MV_Model.h"

#include <fstream>

using MV::Pipeline;

Pipeline::Pipeline(
	Device& device,
	const std::string& vertFilePath,
	const std::string& fragFilePath,
	const PipelineConfigInfo& configInfo) :
	mDevice(device)
{
	CreateGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
}

Pipeline::~Pipeline()
{
	vkDestroyShaderModule(mDevice.GetDevice(), mVertShaderModule, nullptr);
	vkDestroyShaderModule(mDevice.GetDevice(), mFragShaderModule, nullptr);
	vkDestroyPipeline(mDevice.GetDevice(), mGraphicsPipeline, nullptr);
}

void Pipeline::DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
{
	configInfo.mInputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	configInfo.mInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	configInfo.mInputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	configInfo.mViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	configInfo.mViewportInfo.viewportCount = 1;
	configInfo.mViewportInfo.pViewports = nullptr;
	configInfo.mViewportInfo.scissorCount = 1;
	configInfo.mViewportInfo.pScissors = nullptr;

	configInfo.mRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	configInfo.mRasterizationInfo.depthClampEnable = VK_FALSE;
	configInfo.mRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	configInfo.mRasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	configInfo.mRasterizationInfo.lineWidth = 1.0f;
	configInfo.mRasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	configInfo.mRasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	configInfo.mRasterizationInfo.depthBiasEnable = VK_FALSE;

	configInfo.mMultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	configInfo.mMultisampleInfo.sampleShadingEnable = VK_FALSE;
	configInfo.mMultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	configInfo.mColorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	configInfo.mColorBlendAttachment.blendEnable = VK_FALSE;

	configInfo.mColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	configInfo.mColorBlendInfo.logicOpEnable = VK_FALSE;
	configInfo.mColorBlendInfo.attachmentCount = 1;
	configInfo.mColorBlendInfo.pAttachments = &configInfo.mColorBlendAttachment;

	configInfo.mDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	configInfo.mDepthStencilInfo.depthTestEnable = VK_TRUE;
	configInfo.mDepthStencilInfo.depthWriteEnable = VK_TRUE;
	configInfo.mDepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	configInfo.mDepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	configInfo.mDepthStencilInfo.stencilTestEnable = VK_FALSE; 

	configInfo.mDynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	configInfo.mDynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	configInfo.mDynamicStateInfo.pDynamicStates = configInfo.mDynamicStateEnables.data();
	configInfo.mDynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.mDynamicStateEnables.size());
	configInfo.mDynamicStateInfo.flags = 0;
	configInfo.mDynamicStateInfo.pNext = nullptr;
}

void Pipeline::Bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
}

std::vector<char> Pipeline::ReadFile(const std::string& path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	mage_ensure(file.is_open());

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	return buffer;
}

void Pipeline::CreateGraphicsPipeline(
	const std::string& vertFilePath,
	const std::string& fragFilePath,
	const PipelineConfigInfo& configInfo)
{
	mage_check(configInfo.mPipelineLayout != VK_NULL_HANDLE);
	mage_check(configInfo.mRenderPass != VK_NULL_HANDLE);

	std::vector<char> vertCode = ReadFile(vertFilePath);
	std::vector<char> fragCode = ReadFile(fragFilePath);

	CreateShaderModule(vertCode, &mVertShaderModule);
	CreateShaderModule(fragCode, &mFragShaderModule);

	VkPipelineShaderStageCreateInfo shaderStages[2];

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = mVertShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = mFragShaderModule;
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].pSpecializationInfo = nullptr;

	const std::vector<VkVertexInputBindingDescription> bindingDescriptions = Model::Vertex::GetBindingDescriptions();
	const std::vector<VkVertexInputAttributeDescription> attributeDescriptions = Model::Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &configInfo.mInputAssemblyInfo;
	pipelineInfo.pViewportState = &configInfo.mViewportInfo;
	pipelineInfo.pRasterizationState = &configInfo.mRasterizationInfo;
	pipelineInfo.pMultisampleState = &configInfo.mMultisampleInfo;
	pipelineInfo.pColorBlendState = &configInfo.mColorBlendInfo;
	pipelineInfo.pDepthStencilState = &configInfo.mDepthStencilInfo;
	pipelineInfo.pDynamicState = &configInfo.mDynamicStateInfo;

	pipelineInfo.layout = configInfo.mPipelineLayout;
	pipelineInfo.renderPass = configInfo.mRenderPass;
	pipelineInfo.subpass = configInfo.mSubpass;

	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	mage_check(vkCreateGraphicsPipelines(mDevice.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) == VK_SUCCESS);
}

void Pipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	mage_check(vkCreateShaderModule(mDevice.GetDevice(), &createInfo, nullptr, shaderModule) == VK_SUCCESS);
}
