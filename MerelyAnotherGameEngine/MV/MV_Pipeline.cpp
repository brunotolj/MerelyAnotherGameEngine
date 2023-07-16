#include "MV/MV_Pipeline.h"
#include "MV/MV_Model.h"
#include "Asserts.h"

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
	configInfo.InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	configInfo.InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	configInfo.InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	configInfo.ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	configInfo.ViewportInfo.viewportCount = 1;
	configInfo.ViewportInfo.pViewports = nullptr;
	configInfo.ViewportInfo.scissorCount = 1;
	configInfo.ViewportInfo.pScissors = nullptr;

	configInfo.RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	configInfo.RasterizationInfo.depthClampEnable = VK_FALSE;
	configInfo.RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	configInfo.RasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	configInfo.RasterizationInfo.lineWidth = 1.0f;
	configInfo.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	configInfo.RasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	configInfo.RasterizationInfo.depthBiasEnable = VK_FALSE;

	configInfo.MultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	configInfo.MultisampleInfo.sampleShadingEnable = VK_FALSE;
	configInfo.MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	configInfo.ColorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	configInfo.ColorBlendAttachment.blendEnable = VK_FALSE;

	configInfo.ColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	configInfo.ColorBlendInfo.logicOpEnable = VK_FALSE;
	configInfo.ColorBlendInfo.attachmentCount = 1;
	configInfo.ColorBlendInfo.pAttachments = &configInfo.ColorBlendAttachment;

	configInfo.DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	configInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
	configInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;
	configInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	configInfo.DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	configInfo.DepthStencilInfo.stencilTestEnable = VK_FALSE; 

	configInfo.DynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	configInfo.DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	configInfo.DynamicStateInfo.pDynamicStates = configInfo.DynamicStateEnables.data();
	configInfo.DynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.DynamicStateEnables.size());
	configInfo.DynamicStateInfo.flags = 0;
	configInfo.DynamicStateInfo.pNext = nullptr;
}

void Pipeline::Bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
}

std::vector<char> Pipeline::ReadFile(const std::string& path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	ensure(file.is_open());

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
	check(configInfo.PipelineLayout != VK_NULL_HANDLE);
	check(configInfo.RenderPass != VK_NULL_HANDLE);

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
	pipelineInfo.pInputAssemblyState = &configInfo.InputAssemblyInfo;
	pipelineInfo.pViewportState = &configInfo.ViewportInfo;
	pipelineInfo.pRasterizationState = &configInfo.RasterizationInfo;
	pipelineInfo.pMultisampleState = &configInfo.MultisampleInfo;
	pipelineInfo.pColorBlendState = &configInfo.ColorBlendInfo;
	pipelineInfo.pDepthStencilState = &configInfo.DepthStencilInfo;
	pipelineInfo.pDynamicState = &configInfo.DynamicStateInfo;

	pipelineInfo.layout = configInfo.PipelineLayout;
	pipelineInfo.renderPass = configInfo.RenderPass;
	pipelineInfo.subpass = configInfo.Subpass;

	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	check(vkCreateGraphicsPipelines(mDevice.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) == VK_SUCCESS);
}

void Pipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	check(vkCreateShaderModule(mDevice.GetDevice(), &createInfo, nullptr, shaderModule) == VK_SUCCESS);
}