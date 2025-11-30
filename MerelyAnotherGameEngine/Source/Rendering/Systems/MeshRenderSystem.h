#pragma once

#include "Core/NonCopyable.h"

#include <memory>
#include <set>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Buffer;
class Device;
class DescriptorPool;
class DescriptorSetLayout;
class Model;
class Pipeline;
class Renderer;
class Texture;

struct MeshUBO
{
	alignas(16) glm::mat4 CameraTransform;
	alignas(16) glm::vec4 LightDirectionAndAmbient;
};

struct MeshRenderData
{
	const Model* Mesh;
	glm::mat4 Transform;
	glm::vec3 Color;
	uint32_t TextureIndex;
};

struct SceneRenderData
{
	glm::mat4 ViewTransform;
	glm::mat4 ProjectionTransform;

	glm::vec3 LightDirection;
	float AmbientLightIntensity;

	std::vector<MeshRenderData> Meshes;
};

class MeshRenderSystem : public NonCopyableClass
{
	struct PushConstantData
	{
		alignas(16) glm::mat4 Transform{ 1.0f };
		alignas(16) glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 0.0f };
	};

public:
	MeshRenderSystem(Device& device, Renderer& renderer, const std::vector<std::string>& texturePaths);

	~MeshRenderSystem();

	float GetAspectRatio() const;

	void RenderMeshes(VkCommandBuffer commandBuffer, const SceneRenderData& data);

private:
	Device& mDevice;

	std::unique_ptr<DescriptorPool> mDescriptorPool;

	Renderer& mRenderer;

	std::unique_ptr<Pipeline> mPipeline;

	std::vector<std::unique_ptr<Buffer>> mUniformBuffers;

	std::vector<std::unique_ptr<Texture>> mTextures;

	std::unique_ptr<DescriptorSetLayout> mUniformBufferDescriptorSetLayout;

	std::unique_ptr<DescriptorSetLayout> mTextureDescriptorSetLayout;

	std::vector<VkDescriptorSet> mDescriptorSets;

	VkPipelineLayout mPipelineLayout;

	void CreatePipelineLayout(VkDescriptorSetLayout uniformBufferDescriptorSetLayout, VkDescriptorSetLayout textureDescriptorSetLayout);

	void CreatePipeline(VkRenderPass renderPass);
};
