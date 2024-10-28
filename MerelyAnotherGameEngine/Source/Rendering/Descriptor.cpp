#include "Core/Asserts.h"
#include "Rendering/Descriptor.h"

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::AddBinding(
	uint32_t binding,
	VkDescriptorType descriptorType,
	VkShaderStageFlags stageFlags,
	uint32_t count)
{
	mage_check(mBindings.count(binding) == 0);
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = descriptorType;
	layoutBinding.descriptorCount = count;
	layoutBinding.stageFlags = stageFlags;
	mBindings[binding] = layoutBinding;
	return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::Build() const
{
	return std::make_unique<DescriptorSetLayout>(mDevice, mBindings);
}

DescriptorSetLayout::DescriptorSetLayout(Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
	: mDevice{ device }, mBindings{ bindings }
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
	for (const auto& [key, value] : bindings)
	{
		setLayoutBindings.push_back(value);
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
	
	mage_check(vkCreateDescriptorSetLayout(mDevice.GetDevice(), &descriptorSetLayoutInfo, nullptr, &mDescriptorSetLayout) == VK_SUCCESS);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(mDevice.GetDevice(), mDescriptorSetLayout, nullptr);
}

DescriptorPool::Builder& DescriptorPool::Builder::AddPoolSize(VkDescriptorType descriptorType, uint32_t count)
{
	mPoolSizes.push_back({ descriptorType, count });
	return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags)
{
	mPoolFlags = flags;
	return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::SetMaxSets(uint32_t count)
{
	mMaxSets = count;
	return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::Build() const
{
	return std::make_unique<DescriptorPool>(mDevice, mMaxSets, mPoolFlags, mPoolSizes);
}

DescriptorPool::DescriptorPool(
	Device& device,
	uint32_t maxSets,
	VkDescriptorPoolCreateFlags poolFlags,
	const std::vector<VkDescriptorPoolSize>& poolSizes)
	: mDevice(device)
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = maxSets;
	descriptorPoolInfo.flags = poolFlags;

	mage_check(vkCreateDescriptorPool(mDevice.GetDevice(), &descriptorPoolInfo, nullptr, &mDescriptorPool) == VK_SUCCESS);
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(mDevice.GetDevice(), mDescriptorPool, nullptr);
}

bool DescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.pSetLayouts = &descriptorSetLayout;
	allocInfo.descriptorSetCount = 1;

	// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
	// a new pool whenever an old pool fills up. But this is beyond our current scope
	return vkAllocateDescriptorSets(mDevice.GetDevice(), &allocInfo, &descriptor) == VK_SUCCESS;
}

void DescriptorPool::FreeDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets) const
{
	vkFreeDescriptorSets(
		mDevice.GetDevice(),
		mDescriptorPool,
		static_cast<uint32_t>(descriptorSets.size()),
		descriptorSets.data());
}

void DescriptorPool::ResetPool()
{
	vkResetDescriptorPool(mDevice.GetDevice(), mDescriptorPool, 0);
}

DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool)
	: mSetLayout(setLayout), mPool(pool) {}

DescriptorWriter& DescriptorWriter::WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
{
	mage_check(mSetLayout.mBindings.count(binding) == 1);

	VkDescriptorSetLayoutBinding& bindingDescription = mSetLayout.mBindings[binding];
	mage_check(bindingDescription.descriptorCount == 1);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pBufferInfo = bufferInfo;
	write.descriptorCount = 1;

	mWrites.push_back(write);
	return *this;
}

DescriptorWriter& DescriptorWriter::WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo)
{
	mage_check(mSetLayout.mBindings.count(binding) == 1);

	VkDescriptorSetLayoutBinding& bindingDescription = mSetLayout.mBindings[binding];
	mage_check(bindingDescription.descriptorCount == 1);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pImageInfo = imageInfo;
	write.descriptorCount = 1;

	mWrites.push_back(write);
	return *this;
}

bool DescriptorWriter::Build(VkDescriptorSet& set)
{
	if (!mPool.AllocateDescriptorSet(mSetLayout.GetDescriptorSetLayout(), set))
		return false;

	Overwrite(set);
	return true;
}

void DescriptorWriter::Overwrite(VkDescriptorSet& set)
{
	for (VkWriteDescriptorSet& write : mWrites)
		write.dstSet = set;

	vkUpdateDescriptorSets(mPool.mDevice.GetDevice(), static_cast<uint32_t>(mWrites.size()), mWrites.data(), 0, nullptr);
}
