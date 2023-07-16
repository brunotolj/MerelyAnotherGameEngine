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
			alignas(64) glm::mat4 pubTransform{1.0f};
			alignas(16) glm::vec3 pubColor{0.0f};
		};

	public:
		TestRenderSystem(Device& device, VkRenderPass renderPass);

		~TestRenderSystem();

		void RenderObjects(VkCommandBuffer commandBuffer, const std::vector<std::shared_ptr<Object>>& objects);

	private:
		MV::Device& privDevice;

		std::unique_ptr<MV::Pipeline> privPipeline;
		VkPipelineLayout privPipelineLayout;

		void CreatePipelineLayout();

		void CreatePipeline(VkRenderPass renderPass);
	};
}
