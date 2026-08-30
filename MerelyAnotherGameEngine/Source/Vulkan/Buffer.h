#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	class Buffer : public NonMovable
	{
	public:
		struct CreateInfo
		{
			vk::DeviceSize Size;
			vk::BufferUsageFlags UsageFlags;
			vk::MemoryPropertyFlags MemoryFlags;
		};

		Buffer(nullptr_t) {};
		Buffer(CreateInfo const& inCreateInfo);

		void Create(CreateInfo const& inCreateInfo);

		void Map();
		void Unmap();

		void Write(void* inData, vk::DeviceSize inSize) const;
		void Flush() const;

		vk::raii::Buffer const& GetVkBuffer() const;
		vk::DeviceAddress GetDeviceAddress() const;

		void CopyFromBuffer(vk::CommandBuffer inCommandBuffer, Buffer const& inSrcBuffer) const;
		void BindVertexBuffer(vk::CommandBuffer inCommandBuffer) const;
		void BindIndexBuffer(vk::CommandBuffer inCommandBuffer) const;

	private:
		vk::raii::Buffer mVkBuffer = nullptr;
		vk::raii::DeviceMemory mDeviceMemory = nullptr;

		void* mMappedMemory = nullptr;
		vk::DeviceSize mBufferSize = 0;
		vk::DeviceAddress mDeviceAddress = 0;
	};
}
