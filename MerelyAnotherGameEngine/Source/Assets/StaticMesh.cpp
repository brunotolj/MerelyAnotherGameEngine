#include "Assets/StaticMesh.h"
#include "Engine/Engine.h"

mage::Array<vk::VertexInputBindingDescription> StaticMesh::Vertex::GetBindingDescriptions()
{
	return
	{
		{
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = vk::VertexInputRate::eVertex
		}
	};
}

mage::Array<vk::VertexInputAttributeDescription> StaticMesh::Vertex::GetAttributeDescriptions()
{
	return
	{
		{
			.location = 0,
			.binding = 0,
			.format = vk::Format::eR32G32B32Sfloat,
			.offset = offsetof(Vertex, Position)
		},
		{
			.location = 1,
			.binding = 0,
			.format = vk::Format::eR32G32B32Sfloat,
			.offset = offsetof(Vertex, Normal)
		},
		{
			.location = 2,
			.binding = 0,
			.format = vk::Format::eR32G32Sfloat,
			.offset = offsetof(Vertex, TextureCoords)
		}
	};
}

void StaticMesh::Bind(vk::CommandBuffer inCommandBuffer) const
{
	mVertexBuffer.BindVertexBuffer(inCommandBuffer);

	if (mFaces.GetSize() > 0)
		mIndexBuffer.BindIndexBuffer(inCommandBuffer);
}

void StaticMesh::Draw(vk::CommandBuffer inCommandBuffer) const
{
	if (mFaces.GetSize() > 0)
		inCommandBuffer.drawIndexed(3 * mFaces.GetSize(), 1, 0, 0, 0);
	else
		inCommandBuffer.draw(mVertices.GetSize(), 1, 0, 0);
}

void StaticMesh::CreateVertexBuffer()
{
	vk::DeviceSize dataSize = mVertices.GetSize() * sizeof(Vertex);

	Vulkan::Buffer::CreateInfo stagingBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	Vulkan::Buffer stagingBuffer(stagingBufferCreateInfo);

	stagingBuffer.Map();
	stagingBuffer.Write(mVertices.GetData(), dataSize);

	Vulkan::Buffer::CreateInfo vertexBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	mVertexBuffer.Create(vertexBufferCreateInfo);

	gEngine->mVulkanDevice.SubmitSingleTimeCommands([this, &stagingBuffer](vk::CommandBuffer inCommandBuffer)
	{
		mVertexBuffer.CopyFromBuffer(inCommandBuffer, stagingBuffer);
	});
}

void StaticMesh::CreateIndexBuffer()
{
	u32 indexCount = 3 * mFaces.GetSize();
	if (indexCount == 0)
		return;

	vk::DeviceSize dataSize = indexCount * sizeof(u32);

	Vulkan::Buffer::CreateInfo stagingBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	Vulkan::Buffer stagingBuffer(stagingBufferCreateInfo);

	stagingBuffer.Map();
	stagingBuffer.Write(mFaces.GetData(), dataSize);

	Vulkan::Buffer::CreateInfo indexBufferCreateInfo
	{
		.Size = dataSize,
		.UsageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
	};

	mIndexBuffer.Create(indexBufferCreateInfo);

	gEngine->mVulkanDevice.SubmitSingleTimeCommands([this, &stagingBuffer](vk::CommandBuffer inCommandBuffer)
	{
		mIndexBuffer.CopyFromBuffer(inCommandBuffer, stagingBuffer);
	});
}
