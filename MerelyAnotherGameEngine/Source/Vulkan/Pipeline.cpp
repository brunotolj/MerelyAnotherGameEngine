#include "Vulkan/Pipeline.h"

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>
#include <iostream>

namespace Vulkan
{
	Pipeline& Pipeline::operator=(Pipeline&& inPipeline)
	{
		mDescriptorSetLayouts = std::move(inPipeline.mDescriptorSetLayouts);
		mDescriptorPool = std::move(inPipeline.mDescriptorPool);
		mDescriptorSets = std::move(inPipeline.mDescriptorSets);
		mPipelineLayout = std::move(inPipeline.mPipelineLayout);
		mVkPipeline = std::move(inPipeline.mVkPipeline);

		return *this;
	}

	void Pipeline::Bind(vk::CommandBuffer inCommandBuffer) const
    {
		inCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mVkPipeline);
    }

	void Pipeline::BindDescriptorSet(vk::CommandBuffer inCommandBuffer, vk::BindDescriptorSetsInfo inBindInfo, u32 inDescriptorIndex) const
	{
		inBindInfo.layout = mPipelineLayout;
		inBindInfo.descriptorSetCount = 1;
		inBindInfo.pDescriptorSets = &*mDescriptorSets[inDescriptorIndex];
		inCommandBuffer.bindDescriptorSets2(inBindInfo);
	}

	void Pipeline::PushConstants(vk::CommandBuffer inCommandBuffer, vk::PushConstantsInfo inPushInfo) const
	{
		inPushInfo.layout = mPipelineLayout;
		inCommandBuffer.pushConstants2(inPushInfo);
	}

	void Pipeline::UpdateDescriptorSet(vk::WriteDescriptorSet inWrite, u32 inDescriptorSetIndex) const
	{
		inWrite.dstSet = mDescriptorSets[inDescriptorSetIndex];
		mVkPipeline.getDevice().updateDescriptorSets(inWrite, {});
	}

	SpirVBinary CompileSPIRV(glslang_stage_t inShaderStage, mage::StringView inShaderSource, mage::StringView inShaderName)
	{
		const glslang_input_t input
		{
			.language = GLSLANG_SOURCE_GLSL,
			.stage = inShaderStage,
			.client = GLSLANG_CLIENT_VULKAN,
			.client_version = GLSLANG_TARGET_VULKAN_1_4,
			.target_language = GLSLANG_TARGET_SPV,
			.target_language_version = GLSLANG_TARGET_SPV_1_6,
			.code = inShaderSource.GetCString(),
			.default_version = 100,
			.default_profile = GLSLANG_NO_PROFILE,
			.force_default_version_and_profile = false,
			.forward_compatible = false,
			.messages = GLSLANG_MSG_DEFAULT_BIT,
			.resource = glslang_default_resource()
		};

		glslang_shader_t* shader = glslang_shader_create(&input);

		if (!glslang_shader_preprocess(shader, &input))
		{
			std::cout << "GLSL preprocessing failed: " << inShaderName.GetCString() << '\n';
			std::cout << glslang_shader_get_info_log(shader) << '\n';
			std::cout << glslang_shader_get_info_debug_log(shader) << '\n';
			std::cout << input.code << '\n';

			glslang_shader_delete(shader);
			return SpirVBinary();
		}

		if (!glslang_shader_parse(shader, &input))
		{
			std::cout << "GLSL parsing failed: " << inShaderName.GetCString() << '\n';
			std::cout << glslang_shader_get_info_log(shader) << '\n';
			std::cout << glslang_shader_get_info_debug_log(shader) << '\n';
			std::cout << glslang_shader_get_preprocessed_code(shader) << '\n';

			glslang_shader_delete(shader);
			return SpirVBinary();
		}

		glslang_program_t* program = glslang_program_create();
		glslang_program_add_shader(program, shader);

		if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
		{
			std::cout << "GLSL linking failed: " << inShaderName.GetCString() << '\n';
			std::cout << glslang_program_get_info_log(program) << '\n';
			std::cout << glslang_program_get_info_debug_log(program) << '\n';

			glslang_program_delete(program);
			glslang_shader_delete(shader);
			return SpirVBinary();
		}

		glslang_program_SPIRV_generate(program, inShaderStage);

		SpirVBinary binary;
		binary.ResizeUninitialized(u32(glslang_program_SPIRV_get_size(program)));
		glslang_program_SPIRV_get(program, binary.GetData());

		cstr spirv_messages = glslang_program_SPIRV_get_messages(program);
		if (spirv_messages)
			std::cout << "(" << inShaderName.GetCString() << ") " << spirv_messages << '\n';

		glslang_program_delete(program);
		glslang_shader_delete(shader);

		return binary;
	}
}
