#pragma once

#include "Rendering/Device.h"

class Buffer : public NonCopyableClass
{
public:
	Buffer(
		Device& device,
		VkDeviceSize instanceSize,
		uint32_t instanceCount,
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags,
		VkDeviceSize minOffsetAlignment = 1);

	~Buffer();

	VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	void Unmap();

	void WriteToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

	void WriteToIndex(void* data, int index);
	VkResult FlushIndex(int index);
	VkDescriptorBufferInfo DescriptorInfoForIndex(int index);
	VkResult InvalidateIndex(int index);

	VkBuffer GetBuffer() const { return mBuffer; }
	void* GetMappedMemory() const { return mMapped; }
	uint32_t GetInstanceCount() const { return mInstanceCount; }
	VkDeviceSize GetInstanceSize() const { return mInstanceSize; }
	VkDeviceSize GetAlignmentSize() const { return mInstanceSize; }
	VkBufferUsageFlags GetUsageFlags() const { return mUsageFlags; }
	VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return mMemoryPropertyFlags; }
	VkDeviceSize GetBufferSize() const { return mBufferSize; }

private:
	static VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

	Device& mDevice;
	void* mMapped = nullptr;
	VkBuffer mBuffer = VK_NULL_HANDLE;
	VkDeviceMemory mMemory = VK_NULL_HANDLE;

	VkDeviceSize mBufferSize;
	uint32_t mInstanceCount;
	VkDeviceSize mInstanceSize;
	VkDeviceSize mAlignmentSize;
	VkBufferUsageFlags mUsageFlags;
	VkMemoryPropertyFlags mMemoryPropertyFlags;
};
