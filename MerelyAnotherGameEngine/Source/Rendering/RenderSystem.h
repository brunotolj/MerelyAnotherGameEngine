#pragma once

#include "Core/NonCopyable.h"

#include <memory>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Camera;
class Device;
class Pipeline;
class StaticMeshObjectComponent;

class RenderSystem : public NonCopyableClass
{
	struct PushConstantData
	{
		alignas(64) glm::mat4 mTransform{ 1.0f };
		alignas(64) glm::mat4 mNormalMatrix{ 1.0f };
	};

public:
	RenderSystem(Device& device, VkRenderPass renderPass);

	~RenderSystem();

	void SetCamera(const std::shared_ptr<Camera>& camera);

	void RenderScene(VkCommandBuffer commandBuffer);

	void AddStaticMesh(StaticMeshObjectComponent const* staticMesh);

	void RemoveStaticMesh(StaticMeshObjectComponent const* staticMesh);

private:
	Device& mDevice;

	std::unique_ptr<Pipeline> mPipeline;

	std::shared_ptr<Camera> mCamera;

	VkPipelineLayout mPipelineLayout;

	std::set<StaticMeshObjectComponent const*> mStaticMeshes;

	void CreatePipelineLayout();

	void CreatePipeline(VkRenderPass renderPass);
};
