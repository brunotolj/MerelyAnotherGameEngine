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
	privDevice(device)
{
	CreateGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
}

Pipeline::~Pipeline()
{
	vkDestroyShaderModule(privDevice.GetDevice(), privVertShaderModule, nullptr);
	vkDestroyShaderModule(privDevice.GetDevice(), privFragShaderModule, nullptr);
	vkDestroyPipeline(privDevice.GetDevice(), privGraphicsPipeline, nullptr);
}

void Pipeline::DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
{
	configInfo.pubInputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	configInfo.pubInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	configInfo.pubInputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	configInfo.pubViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	configInfo.pubViewportInfo.viewportCount = 1;
	configInfo.pubViewportInfo.pViewports = nullptr;
	configInfo.pubViewportInfo.scissorCount = 1;
	configInfo.pubViewportInfo.pScissors = nullptr;

	configInfo.pubRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	configInfo.pubRasterizationInfo.depthClampEnable = VK_FALSE;
	configInfo.pubRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	configInfo.pubRasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	configInfo.pubRasterizationInfo.lineWidth = 1.0f;
	configInfo.pubRasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	configInfo.pubRasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	configInfo.pubRasterizationInfo.depthBiasEnable = VK_FALSE;

	configInfo.pubMultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	configInfo.pubMultisampleInfo.sampleShadingEnable = VK_FALSE;
	configInfo.pubMultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	configInfo.pubColorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	configInfo.pubColorBlendAttachment.blendEnable = VK_FALSE;

	configInfo.pubColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	configInfo.pubColorBlendInfo.logicOpEnable = VK_FALSE;
	configInfo.pubColorBlendInfo.attachmentCount = 1;
	configInfo.pubColorBlendInfo.pAttachments = &configInfo.pubColorBlendAttachment;

	configInfo.pubDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	configInfo.pubDepthStencilInfo.depthTestEnable = VK_TRUE;
	configInfo.pubDepthStencilInfo.depthWriteEnable = VK_TRUE;
	configInfo.pubDepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	configInfo.pubDepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	configInfo.pubDepthStencilInfo.stencilTestEnable = VK_FALSE; 

	configInfo.pubDynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	configInfo.pubDynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	configInfo.pubDynamicStateInfo.pDynamicStates = configInfo.pubDynamicStateEnables.data();
	configInfo.pubDynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.pubDynamicStateEnables.size());
	configInfo.pubDynamicStateInfo.flags = 0;
	configInfo.pubDynamicStateInfo.pNext = nullptr;
}

void Pipeline::Bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, privGraphicsPipeline);
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
	check(configInfo.pubPipelineLayout != VK_NULL_HANDLE);
	check(configInfo.pubRenderPass != VK_NULL_HANDLE);

	std::vector<char> vertCode = ReadFile(vertFilePath);
	std::vector<char> fragCode = ReadFile(fragFilePath);

	CreateShaderModule(vertCode, &privVertShaderModule);
	CreateShaderModule(fragCode, &privFragShaderModule);

	VkPipelineShaderStageCreateInfo shaderStages[2];

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = privVertShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = privFragShaderModule;
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
	pipelineInfo.pInputAssemblyState = &configInfo.pubInputAssemblyInfo;
	pipelineInfo.pViewportState = &configInfo.pubViewportInfo;
	pipelineInfo.pRasterizationState = &configInfo.pubRasterizationInfo;
	pipelineInfo.pMultisampleState = &configInfo.pubMultisampleInfo;
	pipelineInfo.pColorBlendState = &configInfo.pubColorBlendInfo;
	pipelineInfo.pDepthStencilState = &configInfo.pubDepthStencilInfo;
	pipelineInfo.pDynamicState = &configInfo.pubDynamicStateInfo;

	pipelineInfo.layout = configInfo.pubPipelineLayout;
	pipelineInfo.renderPass = configInfo.pubRenderPass;
	pipelineInfo.subpass = configInfo.pubSubpass;

	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	check(vkCreateGraphicsPipelines(privDevice.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &privGraphicsPipeline) == VK_SUCCESS);
}

void Pipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	check(vkCreateShaderModule(privDevice.GetDevice(), &createInfo, nullptr, shaderModule) == VK_SUCCESS);
}