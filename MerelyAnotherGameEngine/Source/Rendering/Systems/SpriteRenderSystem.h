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

struct SpriteUBO
{
	alignas(16) glm::vec4 ScreenTransform;
};

struct SpriteRenderData
{
	glm::vec2 ScreenCoordsMin;
	glm::vec2 ScreenCoordsMax;
	glm::vec2 TextureCoordsMin;
	glm::vec2 TextureCoordsMax;
	uint32_t TextureIndex;
};

class SpriteRenderSystem : public NonCopyableClass
{
	struct PushConstantData
	{
		alignas(16) glm::vec4 ScreenCoords{ 0.0f, 0.0f, 1.0f, 1.0f };
		alignas(16) glm::vec4 TextureCoords{ 0.0f, 0.0f, 1.0f, 1.0f };
	};

public:
	SpriteRenderSystem(Device& device, Renderer& renderer, const std::vector<std::string>& texturePaths);

	~SpriteRenderSystem();

	float GetAspectRatio() const;

	void RenderSprites(VkCommandBuffer commandBuffer, const std::vector<SpriteRenderData>& data);

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

	std::unique_ptr<Buffer> mVertexBuffer;

	VkPipelineLayout mPipelineLayout;

	void CreatePipelineLayout(VkDescriptorSetLayout uniformBufferDescriptorSetLayout, VkDescriptorSetLayout textureDescriptorSetLayout);
	void CreatePipeline(VkRenderPass renderPass);

	void CreateVertexBuffer();
};
