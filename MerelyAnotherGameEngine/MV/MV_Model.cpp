#include "Core/Asserts.h"
#include "MV/MV_Model.h"

std::vector<VkVertexInputBindingDescription> MV::Model::Vertex::GetBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> MV::Model::Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, mPosition);

	return attributeDescriptions;
}
 
MV::Model::Model(Device& device, const std::vector<Vertex>& vertices) :
	mDevice(device)
{
	CreateVertexBuffers(vertices);
}

MV::Model::~Model()
{
	vkDestroyBuffer(mDevice.GetDevice(), mVertexBuffer, nullptr);
	vkFreeMemory(mDevice.GetDevice(), mVertexBufferMemory, nullptr);
}

void MV::Model::Bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[] = { mVertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void MV::Model::Draw(VkCommandBuffer commandBuffer)
{
	vkCmdDraw(commandBuffer, mVertexCount, 1, 0, 0);
}

void MV::Model::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
	mVertexCount = static_cast<uint32_t>(vertices.size());
	ensure(mVertexCount >= 3);

	VkDeviceSize bufferSize = sizeof(Vertex) * mVertexCount;

	mDevice.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		mVertexBuffer,
		mVertexBufferMemory);

	void* data;
	vkMapMemory(mDevice.GetDevice(), mVertexBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), bufferSize);
	vkUnmapMemory(mDevice.GetDevice(), mVertexBufferMemory);
}
