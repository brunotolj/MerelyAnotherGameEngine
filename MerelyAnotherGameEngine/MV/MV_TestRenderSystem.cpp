#include "MV/MV_TestRenderSystem.h"
#include "Asserts.h"

MV::TestRenderSystem::TestRenderSystem(Device& device, VkRenderPass renderPass) :
	privDevice(device)
{
	CreatePipelineLayout();
	CreatePipeline(renderPass);
}

MV::TestRenderSystem::~TestRenderSystem()
{
	vkDestroyPipelineLayout(privDevice.GetDevice(), privPipelineLayout, nullptr);
}

void MV::TestRenderSystem::RenderObjects(VkCommandBuffer commandBuffer, const std::vector<std::shared_ptr<Object>>& objects)
{
	privPipeline->Bind(commandBuffer);

	for (const std::shared_ptr<Object>& object : objects)
	{
		PushConstantData push;
		push.pubTransform = object->pubTransform.Matrix();
		push.pubColor = object->pubColor;

		vkCmdPushConstants(
			commandBuffer,
			privPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstantData),
			&push);

		check(object->pubModel != nullptr);

		object->pubModel->Bind(commandBuffer);
		object->pubModel->Draw(commandBuffer);
	}
}

void MV::TestRenderSystem::CreatePipelineLayout()
{
	VkPushConstantRange pushConstantRange;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	check(vkCreatePipelineLayout(privDevice.GetDevice(), &pipelineLayoutInfo, nullptr, &privPipelineLayout) == VK_SUCCESS);
}

void MV::TestRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
	check(privPipelineLayout != nullptr);

	PipelineConfigInfo pipelineConfig{};
	Pipeline::DefaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.pubRenderPass = renderPass;
	pipelineConfig.pubPipelineLayout = privPipelineLayout;
	privPipeline = std::make_unique<MV::Pipeline>(
		privDevice,
		"../shaders/SimpleShader.vert.spv",
		"../shaders/SimpleShader.frag.spv",
		pipelineConfig);
}
