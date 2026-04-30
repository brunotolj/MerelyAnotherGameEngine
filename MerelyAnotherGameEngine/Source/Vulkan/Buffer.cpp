#include "Vulkan/Buffer.h"

namespace Vulkan
{
    Buffer& Buffer::operator=(Buffer&& inBuffer)
    {
		mVkBuffer = std::move(inBuffer.mVkBuffer);
		mDeviceMemory = std::move(inBuffer.mDeviceMemory);
		std::swap(mMappedMemory, inBuffer.mMappedMemory);
		std::swap(mBufferSize, inBuffer.mBufferSize);

		return *this;
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

	vk::DescriptorBufferInfo Buffer::GetDescriptorInfo() const
	{
		return vk::DescriptorBufferInfo
		{
			.buffer = mVkBuffer,
			.range = mBufferSize
		};
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
