#include "MV/MV_TestRenderSystem.h"
#include "Asserts.h"

MV::TestRenderSystem::TestRenderSystem(Device& device, VkRenderPass renderPass) :
	mDevice(device)
{
	CreatePipelineLayout();
	CreatePipeline(renderPass);
}

MV::TestRenderSystem::~TestRenderSystem()
{
	vkDestroyPipelineLayout(mDevice.GetDevice(), mPipelineLayout, nullptr);
}

void MV::TestRenderSystem::RenderObjects(VkCommandBuffer commandBuffer, const std::vector<std::shared_ptr<Object>>& objects)
{
	mPipeline->Bind(commandBuffer);

	for (const std::shared_ptr<Object>& object : objects)
	{
		PushConstantData push;
		push.Transform = object->transform.Matrix();
		push.Color = object->color;

		vkCmdPushConstants(
			commandBuffer,
			mPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstantData),
			&push);

		check(object->model != nullptr);

		object->model->Bind(commandBuffer);
		object->model->Draw(commandBuffer);
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

	check(vkCreatePipelineLayout(mDevice.GetDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) == VK_SUCCESS);
}

void MV::TestRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
	check(mPipelineLayout != nullptr);

	PipelineConfigInfo pipelineConfig{};
	Pipeline::DefaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.RenderPass = renderPass;
	pipelineConfig.PipelineLayout = mPipelineLayout;
	mPipeline = std::make_unique<MV::Pipeline>(
		mDevice,
		"../shaders/SimpleShader.vert.spv",
		"../shaders/SimpleShader.frag.spv",
		pipelineConfig);
}
