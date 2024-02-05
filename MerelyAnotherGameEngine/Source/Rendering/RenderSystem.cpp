#include "Rendering/RenderSystem.h"
#include "Core/Asserts.h"
#include "Rendering/Camera.h"
#include "Rendering/Model.h"
#include "Rendering/Pipeline.h"

RenderSystem::RenderSystem(Device& device, VkRenderPass renderPass) :
	mDevice(device)
{
	CreatePipelineLayout();
	CreatePipeline(renderPass);
}

RenderSystem::~RenderSystem()
{
	vkDestroyPipelineLayout(mDevice.GetDevice(), mPipelineLayout, nullptr);
}

void RenderSystem::SetCamera(const std::shared_ptr<Camera>& camera)
{
	mCamera = camera;
}

void RenderSystem::RenderScene(VkCommandBuffer commandBuffer)
{
	mage_check(mCamera != nullptr);

	mPipeline->Bind(commandBuffer);

	const glm::mat4 cameraTransform = mCamera->GetProjectionTransform() * mCamera->GetViewTransform();

	for (Renderable const* renderable : mRenderables)
	{
		const glm::mat4 transform = renderable->GetTransform().Matrix();
		const glm::vec4 color = glm::vec4(renderable->GetColor(), 1.0f);

		PushConstantData push;
		push.NormalMatrix = transform;
		push.NormalMatrix[3] = color;
		push.Transform = cameraTransform * transform;

		vkCmdPushConstants(
			commandBuffer,
			mPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstantData),
			&push);

		renderable->Bind(commandBuffer);
		renderable->Draw(commandBuffer);
	}
}

void RenderSystem::AddRenderable(Renderable const* renderable)
{
	mRenderables.insert(renderable);
}

void RenderSystem::RemoveRenderable(Renderable const* renderable)
{
	mRenderables.erase(renderable);
}

void RenderSystem::CreatePipelineLayout()
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

	mage_check(vkCreatePipelineLayout(mDevice.GetDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) == VK_SUCCESS);
}

void RenderSystem::CreatePipeline(VkRenderPass renderPass)
{
	mage_check(mPipelineLayout != nullptr);

	PipelineConfigInfo pipelineConfig{};
	Pipeline::DefaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.mRenderPass = renderPass;
	pipelineConfig.mPipelineLayout = mPipelineLayout;
	mPipeline = std::make_unique<Pipeline>(
		mDevice,
		"Source/Shaders/SimpleShader.vert.spv",
		"Source/Shaders/SimpleShader.frag.spv",
		pipelineConfig);
}
