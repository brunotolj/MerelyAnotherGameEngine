#pragma once

#include "Core/NonCopyable.h"
#include "Rendering/Device.h"

#include "glslang/Include/glslang_c_shader_types.h"

#include <string>
#include <vector>

struct PipelineConfigInfo : public NonCopyableStruct
{
	PipelineConfigInfo() { memset(this, 0, sizeof(PipelineConfigInfo)); }

	VkPipelineViewportStateCreateInfo mViewportInfo;
	VkPipelineInputAssemblyStateCreateInfo mInputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo mRasterizationInfo;
	VkPipelineMultisampleStateCreateInfo mMultisampleInfo;
	VkPipelineColorBlendAttachmentState mColorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo mColorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo mDepthStencilInfo;
	std::vector<VkDynamicState> mDynamicStateEnables;
	VkPipelineDynamicStateCreateInfo mDynamicStateInfo;
	VkPipelineLayout mPipelineLayout = nullptr;
	VkRenderPass mRenderPass = nullptr;
	uint32_t mSubpass = 0;

	std::vector<VkVertexInputBindingDescription> BindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> AttributeDescriptions;
};

class Pipeline : public NonMovableClass
{
	struct SpirVBinary
	{
		uint32_t* Words = nullptr;
		size_t Size = 0;
	};

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

	static std::vector<char> ReadFile(const std::string& path);

	void CreateGraphicsPipeline(
		const std::string& vertFilePath,
		const std::string& fragFilePath,
		const PipelineConfigInfo& configInfo);

	void CreateShaderModule(SpirVBinary code, VkShaderModule* shaderModule);

	SpirVBinary CompileShaderToSPIRV(glslang_stage_t stage, const char* shaderSource, const char* fileName) const;
};
