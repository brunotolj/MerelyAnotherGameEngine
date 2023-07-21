#pragma once

#include "MV/MV_Device.h"
#include "MV/MV_Object.h"
#include "MV/MV_Pipeline.h"

namespace MV
{
	class TestRenderSystem : public NonCopyableClass
	{
		struct PushConstantData
		{
			alignas(64) glm::mat4 mTransform{1.0f};
			alignas(16) glm::vec3 mColor{0.0f};
		};

	public:
		TestRenderSystem(Device& device, VkRenderPass renderPass);

		~TestRenderSystem();

		void RenderObjects(VkCommandBuffer commandBuffer, const std::vector<std::shared_ptr<Object>>& objects, const glm::mat4& viewTransform);

	private:
		MV::Device& mDevice;

		std::unique_ptr<MV::Pipeline> mPipeline;
		VkPipelineLayout mPipelineLayout;

		void CreatePipelineLayout();

		void CreatePipeline(VkRenderPass renderPass);
	};
}
