#pragma once

#include "Rendering/Device.h"

class Buffer : public NonCopyableClass
{
public:
	Buffer(
		Device& device,
		VkDeviceSize instanceSize,
		u32 instanceCount,
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

	void WriteToIndex(void* data, i32 index);
	VkResult FlushIndex(i32 index);
	VkDescriptorBufferInfo DescriptorInfoForIndex(i32 index);
	VkResult InvalidateIndex(i32 index);

	VkBuffer GetBuffer() const { return mBuffer; }
	void* GetMappedMemory() const { return mMapped; }
	u32 GetInstanceCount() const { return mInstanceCount; }
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
	u32 mInstanceCount;
	VkDeviceSize mInstanceSize;
	VkDeviceSize mAlignmentSize;
	VkBufferUsageFlags mUsageFlags;
	VkMemoryPropertyFlags mMemoryPropertyFlags;
};
