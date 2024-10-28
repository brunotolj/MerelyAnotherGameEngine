#pragma once

#include "Rendering/Device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

class DescriptorSetLayout : public NonCopyableClass
{
	friend class DescriptorWriter;

public:
	class Builder
	{
	public:
		Builder(Device& device) : mDevice(device) {}

		Builder& AddBinding(
			uint32_t binding,
			VkDescriptorType descriptorType,
			VkShaderStageFlags stageFlags,
			uint32_t count = 1);

		std::unique_ptr<DescriptorSetLayout> Build() const;

	private:
		Device& mDevice;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings{};
	};

	DescriptorSetLayout(Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
	~DescriptorSetLayout();

	VkDescriptorSetLayout GetDescriptorSetLayout() const { return mDescriptorSetLayout; }

private:
	Device& mDevice;
	VkDescriptorSetLayout mDescriptorSetLayout;
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings;
};

class DescriptorPool : public NonCopyableClass
{
	friend class DescriptorWriter;

public:
	class Builder
	{
	public:
		Builder(Device& device) : mDevice(device) {}

		Builder& AddPoolSize(VkDescriptorType descriptorType, uint32_t count);
		Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
		Builder& SetMaxSets(uint32_t count);
		std::unique_ptr<DescriptorPool> Build() const;

	private:
		Device& mDevice;
		std::vector<VkDescriptorPoolSize> mPoolSizes{};
		uint32_t mMaxSets = 1000;
		VkDescriptorPoolCreateFlags mPoolFlags = 0;
	};

	DescriptorPool(
		Device& device,
		uint32_t maxSets,
		VkDescriptorPoolCreateFlags poolFlags,
		const std::vector<VkDescriptorPoolSize>& poolSizes);

	~DescriptorPool();

	bool AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

	void FreeDescriptorSets(std::vector<VkDescriptorSet>& descriptors) const;

	void ResetPool();

private:
	Device& mDevice;
	VkDescriptorPool mDescriptorPool;
};

class DescriptorWriter
{
public:
	DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

	DescriptorWriter& WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
	DescriptorWriter& WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

	bool Build(VkDescriptorSet& set);
	void Overwrite(VkDescriptorSet& set);

private:
	DescriptorSetLayout& mSetLayout;
	DescriptorPool& mPool;
	std::vector<VkWriteDescriptorSet> mWrites;
};
