#pragma once

#include "Core/NonCopyable.h"
#include "Rendering/Buffer.h"
#include "Rendering/RenderCommon.h"

#include <memory>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Device;
class DescriptorPool;
class DescriptorSetLayout;
class Pipeline;
class Renderer;

struct GlobalUBO
{
	alignas(16) glm::mat4 CameraTransform;
	alignas(16) glm::vec4 LightDirectionAndAmbient;
};

class RenderSystem : public NonCopyableClass
{
	struct PushConstantData
	{
		alignas(16) glm::mat4 Transform{ 1.0f };
		alignas(16) glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 0.0f };
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

	std::unique_ptr<DescriptorPool> mDescriptorPool;

	Renderer& mRenderer;

	std::unique_ptr<Pipeline> mPipeline;

	std::vector<std::unique_ptr<Buffer>> mUniformBuffers;

	std::unique_ptr<DescriptorSetLayout> mDescriptorSetLayout;

	std::vector<VkDescriptorSet> mDescriptorSets;

	ICamera const* mCamera;

	VkPipelineLayout mPipelineLayout;

	std::set<IRenderable const*> mRenderables;

	void CreatePipelineLayout(VkDescriptorSetLayout descriptorSetLayout);

	void CreatePipeline(VkRenderPass renderPass);
};
