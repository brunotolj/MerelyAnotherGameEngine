#pragma once

#include "Core/NonCopyable.h"
#include "Rendering/RenderCommon.h"

#include <memory>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Device;
class Pipeline;
class Renderer;

class RenderSystem : public NonCopyableClass
{
	struct PushConstantData
	{
		alignas(64) glm::mat4 Transform{ 1.0f };
		alignas(64) glm::mat4 NormalMatrix{ 1.0f };
	};

public:
	RenderSystem(Device& device, Renderer& renderPass);

	~RenderSystem();

	float GetAspectRatio() const;

	void SetCamera(ICamera const* camera);

	void RenderScene(VkCommandBuffer commandBuffer);

	void AddRenderable(IRenderable const* renderable);

	void RemoveRenderable(IRenderable const* renderable);

private:
	Device& mDevice;

	Renderer& mRenderer;

	std::unique_ptr<Pipeline> mPipeline;

	ICamera const* mCamera;

	VkPipelineLayout mPipelineLayout;

	std::set<IRenderable const*> mRenderables;

	void CreatePipelineLayout();

	void CreatePipeline(VkRenderPass renderPass);
};
