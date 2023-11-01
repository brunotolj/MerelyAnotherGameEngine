#include "Core/Asserts.h"
#include "Core/Utils.h"
#include "Rendering/MV_Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace std
{
	template<>
	struct hash<MV::Model::Vertex>
	{
		size_t operator()(const MV::Model::Vertex& vertex) const
		{
			size_t seed = 0;
			mage::HashCombine(seed, vertex.Position, vertex.Normal, vertex.UV);
			return seed;
		}
	};
}

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
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

	attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position) });
	attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Normal) });
	attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV) });

	return attributeDescriptions;
}

void MV::Model::Builder::LoadModel(const std::string& path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	mage_check(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()));

	Vertices.clear();
	Indices.clear();

	std::unordered_map<Vertex, uint32_t> uniqueVertices;

	for (const tinyobj::shape_t& shape : shapes)
	{
		for (const tinyobj::index_t& index : shape.mesh.indices)
		{
			Vertex vertex{};

			if (index.vertex_index >= 0)
			{
				vertex.Position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]};
			}

			if (index.normal_index >= 0)
			{
				vertex.Normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]};
			}

			if (index.texcoord_index >= 0)
			{
				vertex.UV = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1]};
			}

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(Vertices.size());
				Vertices.push_back(vertex);
			}

			Indices.push_back(uniqueVertices[vertex]);
		}
	}
}
 
MV::Model::Model(Device& device, const Builder& builder) :
	mDevice(device)
{
	CreateVertexBuffers(builder.Vertices);
	CreateIndexBuffers(builder.Indices);
}

MV::Model::~Model()
{
	vkDestroyBuffer(mDevice.GetDevice(), mVertexBuffer, nullptr);
	vkFreeMemory(mDevice.GetDevice(), mVertexBufferMemory, nullptr);

	if (mIndexCount > 0)
	{
		vkDestroyBuffer(mDevice.GetDevice(), mIndexBuffer, nullptr);
		vkFreeMemory(mDevice.GetDevice(), mIndexBufferMemory, nullptr);
	}
}

std::unique_ptr<MV::Model> MV::Model::CreateFromFile(Device& device, const std::string& path)
{
	Builder builder;
	builder.LoadModel(path);

	return std::make_unique<Model>(device, builder);
}

void MV::Model::Bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[] = { mVertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	if (mIndexCount > 0)
	{
		vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}
}

void MV::Model::Draw(VkCommandBuffer commandBuffer)
{
	if (mIndexCount > 0)
	{
		vkCmdDrawIndexed(commandBuffer, mIndexCount, 1, 0, 0, 0);
	}
	else
	{
		vkCmdDraw(commandBuffer, mVertexCount, 1, 0, 0);
	}
}

void MV::Model::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
	mVertexCount = static_cast<uint32_t>(vertices.size());
	mage_ensure(mVertexCount >= 3);

	VkDeviceSize bufferSize = sizeof(Vertex) * mVertexCount;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	mDevice.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	void* data;
	vkMapMemory(mDevice.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), bufferSize);
	vkUnmapMemory(mDevice.GetDevice(), stagingBufferMemory);

	mDevice.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mVertexBuffer,
		mVertexBufferMemory);

	mDevice.CopyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

	vkDestroyBuffer(mDevice.GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(mDevice.GetDevice(), stagingBufferMemory, nullptr);
}

void MV::Model::CreateIndexBuffers(const std::vector<uint32_t>& indices)
{
	mIndexCount = static_cast<uint32_t>(indices.size());
	if (mIndexCount == 0)
	{
		return;
	}

	mage_ensure(mIndexCount >= 3);

	VkDeviceSize bufferSize = sizeof(uint32_t) * mIndexCount;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	mDevice.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	void* data;
	vkMapMemory(mDevice.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), bufferSize);
	vkUnmapMemory(mDevice.GetDevice(), stagingBufferMemory);

	mDevice.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mIndexBuffer,
		mIndexBufferMemory);

	mDevice.CopyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

	vkDestroyBuffer(mDevice.GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(mDevice.GetDevice(), stagingBufferMemory, nullptr);
}
