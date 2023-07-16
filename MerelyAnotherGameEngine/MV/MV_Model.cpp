#include "MV/MV_Model.h"
#include "Asserts.h"

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
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pubPosition);

	return attributeDescriptions;
}
 
MV::Model::Model(Device& device, const std::vector<Vertex>& vertices) :
	privDevice(device)
{
	CreateVertexBuffers(vertices);
}

MV::Model::~Model()
{
	vkDestroyBuffer(privDevice.GetDevice(), privVertexBuffer, nullptr);
	vkFreeMemory(privDevice.GetDevice(), privVertexBufferMemory, nullptr);
}

void MV::Model::Bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[] = { privVertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void MV::Model::Draw(VkCommandBuffer commandBuffer)
{
	vkCmdDraw(commandBuffer, privVertexCount, 1, 0, 0);
}

void MV::Model::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
	privVertexCount = static_cast<uint32_t>(vertices.size());
	ensure(privVertexCount >= 3);

	VkDeviceSize bufferSize = sizeof(Vertex) * privVertexCount;

	privDevice.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		privVertexBuffer,
		privVertexBufferMemory);

	void* data;
	vkMapMemory(privDevice.GetDevice(), privVertexBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), bufferSize);
	vkUnmapMemory(privDevice.GetDevice(), privVertexBufferMemory);
}
