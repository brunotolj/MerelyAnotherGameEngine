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
			u32 binding,
			VkDescriptorType descriptorType,
			VkShaderStageFlags stageFlags,
			u32 count = 1);

		std::unique_ptr<DescriptorSetLayout> Build() const;

	private:
		Device& mDevice;
		std::unordered_map<u32, VkDescriptorSetLayoutBinding> mBindings{};
	};

	DescriptorSetLayout(Device& device, std::unordered_map<u32, VkDescriptorSetLayoutBinding> bindings);
	~DescriptorSetLayout();

	VkDescriptorSetLayout GetDescriptorSetLayout() const { return mDescriptorSetLayout; }

private:
	Device& mDevice;
	VkDescriptorSetLayout mDescriptorSetLayout;
	std::unordered_map<u32, VkDescriptorSetLayoutBinding> mBindings;
};

class DescriptorPool : public NonCopyableClass
{
	friend class DescriptorWriter;

public:
	class Builder
	{
	public:
		Builder(Device& device) : mDevice(device) {}

		Builder& AddPoolSize(VkDescriptorType descriptorType, u32 count);
		Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
		Builder& SetMaxSets(u32 count);
		std::unique_ptr<DescriptorPool> Build() const;

	private:
		Device& mDevice;
		std::vector<VkDescriptorPoolSize> mPoolSizes{};
		u32 mMaxSets = 1000;
		VkDescriptorPoolCreateFlags mPoolFlags = 0;
	};

	DescriptorPool(
		Device& device,
		u32 maxSets,
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

	DescriptorWriter& WriteBuffer(u32 binding, VkDescriptorBufferInfo* bufferInfo);
	DescriptorWriter& WriteImage(u32 binding, VkDescriptorImageInfo* imageInfo);

	bool Build(VkDescriptorSet& set);
	void Overwrite(VkDescriptorSet& set);

private:
	DescriptorSetLayout& mSetLayout;
	DescriptorPool& mPool;
	std::vector<VkWriteDescriptorSet> mWrites;
};
