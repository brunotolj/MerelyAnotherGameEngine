#pragma once

#include "MV/MV_Device.h"
#include "MV/MV_Object.h"

namespace MV
{
	class Camera;
	class Pipeline;

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

		void SetCamera(const std::shared_ptr<Camera>& camera);

		void RenderObjects(VkCommandBuffer commandBuffer, const std::vector<std::shared_ptr<Object>>& objects);

	private:
		Device& mDevice;

		std::unique_ptr<Pipeline> mPipeline;

		std::shared_ptr<Camera> mCamera;

		VkPipelineLayout mPipelineLayout;

		void CreatePipelineLayout();

		void CreatePipeline(VkRenderPass renderPass);
	};
}
