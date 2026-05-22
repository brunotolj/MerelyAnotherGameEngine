#include "Vulkan/Buffer.h"
#include "Engine/Engine.h"

namespace Vulkan
{
    Buffer::Buffer(CreateInfo const& inCreateInfo)
    {
		Create(inCreateInfo);
    }

	void Buffer::Create(CreateInfo const& inCreateInfo)
	{
		vk::raii::Device const& device = gEngine->mVulkanDevice.GetVkDevice();

		vk::BufferCreateInfo bufferCreateInfo
		{
			.size = inCreateInfo.Size,
			.usage = inCreateInfo.UsageFlags | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			.sharingMode = vk::SharingMode::eExclusive
		};

		mVkBuffer = device.createBuffer(bufferCreateInfo);
		mBufferSize = inCreateInfo.Size;

		vk::MemoryRequirements memRequirements = mVkBuffer.getMemoryRequirements();
		u32 memoryTypeIndex = gEngine->mVulkanDevice.SelectMemoryType(memRequirements.memoryTypeBits, inCreateInfo.MemoryFlags);

		vk::MemoryAllocateFlagsInfo memoryAllocFlagsInfo{ .flags = vk::MemoryAllocateFlagBits::eDeviceAddress };

		vk::MemoryAllocateInfo memoryAllocInfo
		{
			.pNext = memoryAllocFlagsInfo,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = memoryTypeIndex
		};

		mDeviceMemory = device.allocateMemory(memoryAllocInfo);
		mVkBuffer.bindMemory(mDeviceMemory, 0);

		mDeviceAddress = device.getBufferAddress({ .buffer = mVkBuffer });
	}

    void Buffer::Map()
	{
		mage_check(mMappedMemory == nullptr);

		mMappedMemory = mDeviceMemory.mapMemory(0, vk::WholeSize);
	}

	void Buffer::Unmap()
	{
		if (mMappedMemory == nullptr)
			return;

		mDeviceMemory.unmapMemory();
		mMappedMemory = nullptr;
	}

	void Buffer::Write(void* inData, vk::DeviceSize inSize) const
	{
		mage_check(mMappedMemory != nullptr);
		memcpy_s(mMappedMemory, mBufferSize, inData, inSize);
	}

    void Buffer::Flush() const
    {
		vk::MappedMemoryRange memoryRange
		{
			.memory = mDeviceMemory,
			.size = VK_WHOLE_SIZE
		};

		mVkBuffer.getDevice().flushMappedMemoryRanges(memoryRange);
    }

	vk::raii::Buffer const& Buffer::GetVkBuffer() const
	{
		return mVkBuffer;
	}

	vk::DeviceAddress Buffer::GetDeviceAddress() const
	{
		return mDeviceAddress;
	}

	void Buffer::CopyFromBuffer(vk::CommandBuffer inCommandBuffer, Buffer const& inSrcBuffer) const
	{
		vk::BufferCopy copyRegion{ .size = mBufferSize };
		inCommandBuffer.copyBuffer(inSrcBuffer.mVkBuffer, mVkBuffer, copyRegion);
	}

	void Buffer::BindVertexBuffer(vk::CommandBuffer inCommandBuffer) const
	{
		inCommandBuffer.bindVertexBuffers(0, *mVkBuffer, { 0 });
	}

	void Buffer::BindIndexBuffer(vk::CommandBuffer inCommandBuffer) const
	{
		inCommandBuffer.bindIndexBuffer(mVkBuffer, 0, vk::IndexType::eUint32);
	}
}
