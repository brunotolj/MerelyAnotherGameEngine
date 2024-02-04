#include "Core/Asserts.h"
#include "Rendering/Camera.h"
#include "Rendering/Model.h"
#include "Rendering/Pipeline.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/StaticMeshObjectComponent.h"

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

	for (StaticMeshObjectComponent const* staticMesh : mStaticMeshes)
	{
		PushConstantData push;
		push.mNormalMatrix = staticMesh->GetTransformMatrix();
		push.mNormalMatrix[3] = glm::vec4(staticMesh->mColor, 1.0f);
		push.mTransform = cameraTransform * staticMesh->GetTransformMatrix();

		vkCmdPushConstants(
			commandBuffer,
			mPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstantData),
			&push);

		mage_check(staticMesh->mModel != nullptr);

		staticMesh->mModel->Bind(commandBuffer);
		staticMesh->mModel->Draw(commandBuffer);
	}
}

void RenderSystem::AddStaticMesh(StaticMeshObjectComponent const* staticMesh)
{
	mStaticMeshes.insert(staticMesh);
}

void RenderSystem::RemoveStaticMesh(StaticMeshObjectComponent const* staticMesh)
{
	mStaticMeshes.erase(staticMesh);
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
