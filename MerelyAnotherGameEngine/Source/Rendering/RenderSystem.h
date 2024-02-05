#pragma once

#include "Core/NonCopyable.h"
#include "Rendering/RenderCommon.h"

#include <memory>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Camera;
class Device;
class Pipeline;

class RenderSystem : public NonCopyableClass
{
	struct PushConstantData
	{
		alignas(64) glm::mat4 Transform{ 1.0f };
		alignas(64) glm::mat4 NormalMatrix{ 1.0f };
	};

public:
	RenderSystem(Device& device, VkRenderPass renderPass);

	~RenderSystem();

	void SetCamera(const std::shared_ptr<Camera>& camera);

	void RenderScene(VkCommandBuffer commandBuffer);

	void AddRenderable(Renderable const* renderable);

	void RemoveRenderable(Renderable const* renderable);

private:
	Device& mDevice;

	std::unique_ptr<Pipeline> mPipeline;

	std::shared_ptr<Camera> mCamera;

	VkPipelineLayout mPipelineLayout;

	std::set<Renderable const*> mRenderables;

	void CreatePipelineLayout();

	void CreatePipeline(VkRenderPass renderPass);
};
