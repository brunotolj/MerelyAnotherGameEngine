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

		VkPipelineViewportStateCreateInfo pubViewportInfo;
		VkPipelineInputAssemblyStateCreateInfo pubInputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo pubRasterizationInfo;
		VkPipelineMultisampleStateCreateInfo pubMultisampleInfo;
		VkPipelineColorBlendAttachmentState pubColorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo pubColorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo pubDepthStencilInfo;
		std::vector<VkDynamicState> pubDynamicStateEnables;
		VkPipelineDynamicStateCreateInfo pubDynamicStateInfo;
		VkPipelineLayout pubPipelineLayout = nullptr;
		VkRenderPass pubRenderPass = nullptr;
		uint32_t pubSubpass = 0;
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
		Device& privDevice;
		VkPipeline privGraphicsPipeline;
		VkShaderModule privVertShaderModule;
		VkShaderModule privFragShaderModule;

		static std::vector<char> ReadFile(const std::string& path);

		void CreateGraphicsPipeline(
			const std::string& vertFilePath,
			const std::string& fragFilePath,
			const PipelineConfigInfo& configInfo);

		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
	};
}
