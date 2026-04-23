#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	struct BufferCreateInfo
	{
		vk::DeviceSize Size;
		vk::BufferUsageFlags UsageFlags;
		vk::MemoryPropertyFlags MemoryFlags;
	};

	class Buffer : public NonCopyableClass
	{
		friend class Renderer;

	public:
		Buffer(nullptr_t) {};
		Buffer(Buffer&& inBuffer) { *this = std::move(inBuffer); };
		Buffer& operator=(Buffer&& inBuffer);

		void Map();
		void Unmap();

		void Write(void* inData, vk::DeviceSize inSize) const;
		void Flush() const;

		vk::DescriptorBufferInfo GetDescriptorInfo() const;

		void BindVertexBuffer(vk::CommandBuffer inCommandBuffer) const;
		void BindIndexBuffer(vk::CommandBuffer inCommandBuffer) const;

	private:
		vk::raii::Buffer mVkBuffer = nullptr;
		vk::raii::DeviceMemory mDeviceMemory = nullptr;

		void* mMappedMemory = nullptr;
		vk::DeviceSize mBufferSize = 0;
	};
}
