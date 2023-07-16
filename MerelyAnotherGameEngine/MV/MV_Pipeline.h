#pragma once

#include "MV/MV_Device.h"
#include "NonCopyable.h"

#include <string>
#include <vector>

namespace MV
{
	struct PipelineConfigInfo : public NonCopyableStruct
	{
		PipelineConfigInfo() { memset(this, 0, sizeof(PipelineConfigInfo)); }

		VkPipelineViewportStateCreateInfo ViewportInfo;
		VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo RasterizationInfo;
		VkPipelineMultisampleStateCreateInfo MultisampleInfo;
		VkPipelineColorBlendAttachmentState ColorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo ColorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo DepthStencilInfo;
		std::vector<VkDynamicState> DynamicStateEnables;
		VkPipelineDynamicStateCreateInfo DynamicStateInfo;
		VkPipelineLayout PipelineLayout = nullptr;
		VkRenderPass RenderPass = nullptr;
		uint32_t Subpass = 0;
	};

	class Pipeline : public NonMovableClass
	{
	public:
		Pipeline(
			Device& device,
			const std::string& vertFilePath,
			const std::string& fragFilePath,
			const PipelineConfigInfo& configInfo);

		~Pipeline();

		static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

		void Bind(VkCommandBuffer commandBuffer);

	private:
		Device& mDevice;
		VkPipeline mGraphicsPipeline;
		VkShaderModule mVertShaderModule;
		VkShaderModule mFragShaderModule;

	private:
		static std::vector<char> ReadFile(const std::string& path);

		void CreateGraphicsPipeline(
			const std::string& vertFilePath,
			const std::string& fragFilePath,
			const PipelineConfigInfo& configInfo);

		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
	};
}
