#include "Core/Asserts.h"
#include "Core/Rotor.h"
#include "Core/Utils.h"
#include "Rendering/Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace std
{
	template<>
	struct hash<Model::Vertex>
	{
		size_t operator()(const Model::Vertex& vertex) const
		{
			size_t seed = 0;
			mage::HashCombine(seed, vertex.Position, vertex.Normal, vertex.UV);
			return seed;
		}
	};
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::GetBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

	attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position) });
	attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Normal) });
	attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV) });

	return attributeDescriptions;
}

void Model::Builder::LoadModel(const std::string& path)
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
				vertex.Position =
				{
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
			}

			if (index.normal_index >= 0)
			{
				vertex.Normal =
				{
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
			}

			if (index.texcoord_index >= 0)
			{
				vertex.UV =
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1]
				};
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
 
void Model::Builder::MakeCube(float halfExtentX, float halfExtentY, float halfExtentZ)
{
	constexpr glm::vec3 positions[8]
	{
		{ -1.0f, -1.0f, -1.0f },
		{  1.0f, -1.0f, -1.0f },
		{ -1.0f,  1.0f, -1.0f },
		{  1.0f,  1.0f, -1.0f },
		{ -1.0f, -1.0f,  1.0f },
		{  1.0f, -1.0f,  1.0f },
		{ -1.0f,  1.0f,  1.0f },
		{  1.0f,  1.0f,  1.0f }
	};

	constexpr glm::vec3 normals[6]
	{
		{ -1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
		{  0.0f, -1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f,  1.0f }
	};

	const glm::vec3 halfExtent(halfExtentX, halfExtentY, halfExtentZ);

	for (int32_t i = 0; i < 6; i++)
	{
		for (int32_t j = 0; j < 8; j++)
		{
			const glm::vec3 test = positions[j] * normals[i];

			if (test.x < 0.0f)
				continue;

			if (test.y < 0.0f)
				continue;

			if (test.z < 0.0f)
				continue;

			Vertices.emplace_back(halfExtent * positions[j], normals[i], glm::vec2(0.0f));
		}

		for (int32_t j = 0; j < 6; j++)
			Indices.push_back(4 * i + (j / 3) + (j % 3));
	}
}

void Model::Builder::MakeSphere(float radius)
{
	constexpr float phi = glm::golden_ratio<float>();
	const float factor = glm::inversesqrt(1.0f + phi * phi);

	constexpr glm::vec3 positions[12]
	{
		{ -1.0f, -phi, 0.0f },
		{  1.0f, -phi, 0.0f },
		{ -1.0f,  phi, 0.0f },
		{  1.0f,  phi, 0.0f },

		{ 0.0f, -1.0f, -phi },
		{ 0.0f,  1.0f, -phi },
		{ 0.0f, -1.0f,  phi },
		{ 0.0f,  1.0f,  phi },

		{ -phi, 0.0f, -1.0f },
		{ -phi, 0.0f,  1.0f },
		{  phi, 0.0f, -1.0f },
		{  phi, 0.0f,  1.0f }
	};

	for (int32_t i = 0; i < 12; i++)
	{
		glm::vec3 pos = factor * positions[i];

		Vertices.emplace_back(radius * pos, pos, glm::vec2(0.0f));
	}

	std::vector<std::pair<uint32_t, uint32_t>> edges;

	for (int32_t i = 0; i < 12; i += 2)
	{
		edges.emplace_back(i, i + 1);
	}

	for (int32_t i = 0; i < 3; i++)
		for (int32_t j = 0; j < 4; j++)
			for (int32_t k = 0; k < 4; k++)
			{
				const int32_t indexA = (4 * i + j) % 12;
				const int32_t indexB = (4 * i + k + 4) % 12;

				const glm::vec3 test = positions[indexA] * positions[indexB];

				if (test.x < 0.0f)
					continue;

				if (test.y < 0.0f)
					continue;

				if (test.z < 0.0f)
					continue;

				if (indexA < indexB)
					edges.emplace_back(indexA, indexB);
				else
					edges.emplace_back(indexB, indexA);
			}

	std::sort(edges.begin(), edges.end());

	for (int32_t subdiv = 0; subdiv < 3; subdiv++)
	{
		std::vector<std::pair<uint32_t, uint32_t>> oldEdges = edges;
		edges.clear();

		const uint32_t indexOffset = static_cast<uint32_t>(Vertices.size());

		for (int32_t i = 0; i < oldEdges.size(); i++)
		{
			glm::vec3 pos = Vertices[oldEdges[i].first].Normal + Vertices[oldEdges[i].second].Normal;
			pos /= glm::length(pos);

			Vertices.emplace_back(radius * pos, pos, glm::vec2(0.0f));

			edges.emplace_back(oldEdges[i].first, i + indexOffset);
			edges.emplace_back(oldEdges[i].second, i + indexOffset);

			for (int32_t j = 0; j < i; j++)
			{
				if (oldEdges[j].second != oldEdges[i].first)
					continue;

				for (int32_t k = j + 1; k < i; k++)
				{
					if (oldEdges[k].first != oldEdges[j].first)
						break;

					if (oldEdges[k].second != oldEdges[i].second)
						continue;

					edges.emplace_back(j + indexOffset, k + indexOffset);
					edges.emplace_back(j + indexOffset, i + indexOffset);
					edges.emplace_back(k + indexOffset, i + indexOffset);
				}
			}
		}

		std::sort(edges.begin(), edges.end());
	}

	for (int32_t i = 0; i < edges.size(); i++)
		for (int32_t j = i + 1; j < edges.size(); j++)
		{
			if (edges[i].first != edges[j].first)
				continue;

			if (!std::binary_search(edges.begin(), edges.end(), std::make_pair(edges[i].second, edges[j].second)))
				continue;

			Indices.push_back(edges[i].first);
			Indices.push_back(edges[i].second);
			Indices.push_back(edges[j].second);
		}
}

void Model::Builder::MakeCylinder(float radius, float halfHeight)
{
	std::vector<std::pair<uint32_t, uint32_t>> edges;

	const glm::vec3 halfHeightVec = glm::vec3(halfHeight, 0.0f, 0.0f);
	const glm::vec3 normalVec = glm::vec3(1.0f, 0.0f, 0.0f);

	for (int32_t i = 0; i < 80; i++)
	{
		const float c = glm::cos(glm::radians(4.5f * i));
		const float s = glm::sin(glm::radians(4.5f * i));
		const glm::vec3 base = glm::vec3(0.0f, c, s);

		Vertices.emplace_back(radius * base + halfHeightVec, base, glm::vec2(0.0f));
		Vertices.emplace_back(radius * base - halfHeightVec, base, glm::vec2(0.0f));

		edges.emplace_back(2 * i, 2 * i + 1);
		edges.emplace_back(i ? 2 * i - 1 : 0, i ? 2 * i : 159);

		edges.emplace_back(0 + (i ? 2 * i - 2 : 0), 0 + (i ? 2 * i : 158));
		edges.emplace_back(1 + (i ? 2 * i - 2 : 0), 1 + (i ? 2 * i : 158));
	}

	for (int32_t i = 0; i < 80; i++)
	{
		const float c = glm::cos(glm::radians(4.5f * i));
		const float s = glm::sin(glm::radians(4.5f * i));
		const glm::vec3 base = glm::vec3(0.0f, c, s);

		Vertices.emplace_back(radius * base + halfHeightVec, normalVec, glm::vec2(0.0f));
		Vertices.emplace_back(radius * base - halfHeightVec, -normalVec, glm::vec2(0.0f));

		edges.emplace_back(160 + (i ? 2 * i - 2 : 0), 160 + (i ? 2 * i : 158));
		edges.emplace_back(161 + (i ? 2 * i - 2 : 0), 161 + (i ? 2 * i : 158));
	}

	for (int32_t i = 0; i < 40; i++)
	{
		const float c = glm::cos(glm::radians(9.0f * i));
		const float s = glm::sin(glm::radians(9.0f * i));
		const glm::vec3 base = glm::vec3(0.0f, c, s);

		Vertices.emplace_back(0.9f * radius * base + halfHeightVec, normalVec, glm::vec2(0.0f));
		Vertices.emplace_back(0.9f * radius * base - halfHeightVec, -normalVec, glm::vec2(0.0f));

		edges.emplace_back(320 + (i ? 2 * i - 2 : 0), 320 + (i ? 2 * i : 78));
		edges.emplace_back(160 + (i ? 4 * i - 2 : 158), 320 + 2 * i);
		edges.emplace_back(160 + 4 * i, 320 + 2 * i);
		edges.emplace_back(160 + 4 * i + 2, 320 + 2 * i);

		edges.emplace_back(321 + (i ? 2 * i - 2 : 0), 321 + (i ? 2 * i : 78));
		edges.emplace_back(161 + (i ? 4 * i - 2 : 158), 321 + 2 * i);
		edges.emplace_back(161 + 4 * i, 321 + 2 * i);
		edges.emplace_back(161 + 4 * i + 2, 321 + 2 * i);
	}

	for (int32_t i = 0; i < 20; i++)
	{
		const float c = glm::cos(glm::radians(18.0f * i));
		const float s = glm::sin(glm::radians(18.0f * i));
		const glm::vec3 base = glm::vec3(0.0f, c, s);

		Vertices.emplace_back(0.75f * radius * base + halfHeightVec, normalVec, glm::vec2(0.0f));
		Vertices.emplace_back(0.75f * radius * base - halfHeightVec, -normalVec, glm::vec2(0.0f));

		edges.emplace_back(400 + (i ? 2 * i - 2 : 0), 400 + (i ? 2 * i : 38));
		edges.emplace_back(320 + (i ? 4 * i - 2 : 78), 400 + 2 * i);
		edges.emplace_back(320 + 4 * i, 400 + 2 * i);
		edges.emplace_back(320 + 4 * i + 2, 400 + 2 * i);

		edges.emplace_back(401 + (i ? 2 * i - 2 : 0), 401 + (i ? 2 * i : 38));
		edges.emplace_back(321 + (i ? 4 * i - 2 : 78), 401 + 2 * i);
		edges.emplace_back(321 + 4 * i, 401 + 2 * i);
		edges.emplace_back(321 + 4 * i + 2, 401 + 2 * i);
	}

	for (int32_t i = 0; i < 10; i++)
	{
		const float c = glm::cos(glm::radians(36.0f * i));
		const float s = glm::sin(glm::radians(36.0f * i));
		const glm::vec3 base = glm::vec3(0.0f, c, s);

		Vertices.emplace_back(0.55f * radius * base + halfHeightVec, normalVec, glm::vec2(0.0f));
		Vertices.emplace_back(0.55f * radius * base - halfHeightVec, -normalVec, glm::vec2(0.0f));

		edges.emplace_back(440 + (i ? 2 * i - 2 : 0), 440 + (i ? 2 * i : 18));
		edges.emplace_back(400 + (i ? 4 * i - 2 : 38), 440 + 2 * i);
		edges.emplace_back(400 + 4 * i, 440 + 2 * i);
		edges.emplace_back(400 + 4 * i + 2, 440 + 2 * i);

		edges.emplace_back(441 + (i ? 2 * i - 2 : 0), 441 + (i ? 2 * i : 18));
		edges.emplace_back(401 + (i ? 4 * i - 2 : 38), 441 + 2 * i);
		edges.emplace_back(401 + 4 * i, 441 + 2 * i);
		edges.emplace_back(401 + 4 * i + 2, 441 + 2 * i);
	}

	for (int32_t i = 0; i < 5; i++)
	{
		const float c = glm::cos(glm::radians(72.0f * i));
		const float s = glm::sin(glm::radians(72.0f * i));
		const glm::vec3 base = glm::vec3(0.0f, c, s);

		Vertices.emplace_back(0.3f * radius * base + halfHeightVec, normalVec, glm::vec2(0.0f));
		Vertices.emplace_back(0.3f * radius * base - halfHeightVec, -normalVec, glm::vec2(0.0f));

		edges.emplace_back(460 + (i ? 2 * i - 2 : 0), 460 + (i ? 2 * i : 8));
		edges.emplace_back(440 + (i ? 4 * i - 2 : 18), 460 + 2 * i);
		edges.emplace_back(440 + 4 * i, 460 + 2 * i);
		edges.emplace_back(440 + 4 * i + 2, 460 + 2 * i);
		edges.emplace_back(460 + 2 * i, 470);

		edges.emplace_back(461 + (i ? 2 * i - 2 : 0), 461 + (i ? 2 * i : 8));
		edges.emplace_back(441 + (i ? 4 * i - 2 : 18), 461 + 2 * i);
		edges.emplace_back(441 + 4 * i, 461 + 2 * i);
		edges.emplace_back(441 + 4 * i + 2, 461 + 2 * i);
		edges.emplace_back(461 + 2 * i, 471);
	}

	Vertices.emplace_back(halfHeightVec, normalVec, glm::vec2(0.0f));
	Vertices.emplace_back(-halfHeightVec, -normalVec, glm::vec2(0.0f));

	std::sort(edges.begin(), edges.end());

	for (int32_t i = 0; i < edges.size(); i++)
		for (int32_t j = i + 1; j < edges.size(); j++)
		{
			if (edges[i].first != edges[j].first)
				continue;

			if (!std::binary_search(edges.begin(), edges.end(), std::make_pair(edges[i].second, edges[j].second)))
				continue;

			Indices.push_back(edges[i].first);
			Indices.push_back(edges[i].second);
			Indices.push_back(edges[j].second);
		}
}

void Model::Builder::MakeCapsule(float radius, float halfHeight)
{
	constexpr float phi = glm::golden_ratio<float>();
	auto reflect = [](glm::vec3 a, glm::vec3 b) -> glm::vec3 { return 2.0f * glm::dot(a, b) * b - a; };

	std::vector<std::pair<uint32_t, uint32_t>> edges;

	const glm::vec3 tip = glm::vec3(1.0f, 0.0f, 0.0f);
	Vertices.emplace_back(radius * tip, tip, glm::vec2(0.0f));

	const glm::vec3 first = glm::vec3(glm::sqrt(1.0f - 1.0f / (glm::sqrt(5.0f) * phi)), 1.0f / glm::sqrt(glm::sqrt(5.0f) * phi), 0.0f);
	Vertices.emplace_back(radius * first, first, glm::vec2(0.0f));
	
	for (int32_t i = 1; i < 5; i++)
	{
		const mage::Rotor rotor = mage::Rotor::FromAxisAndAngle(tip, glm::radians(72.0f * i));
		const glm::vec3 rotated = rotor.Rotate(first);
		Vertices.emplace_back(radius * rotated, rotated, glm::vec2(0.0f));
	}

	const glm::vec3 second = reflect(tip, first);
	Vertices.emplace_back(radius * second, second, glm::vec2(0.0f));

	for (int32_t i = 1; i < 5; i++)
	{
		const mage::Rotor rotor = mage::Rotor::FromAxisAndAngle(tip, glm::radians(72.0f * i));
		const glm::vec3 rotated = rotor.Rotate(second);
		Vertices.emplace_back(radius * rotated, rotated, glm::vec2(0.0f));
	}

	for (int32_t i = 0; i < 5; i++)
	{
		const int32_t j = (i + 1) % 5;
		glm::vec3 pos = Vertices[6 + i].Position + Vertices[6 + j].Position;
		pos /= glm::length(pos);

		Vertices.emplace_back(radius * pos, pos, glm::vec2(0.0f));

		edges.emplace_back(0, 1 + i);
		edges.emplace_back(1 + std::min(i, j), 1 + std::max(i, j));
		edges.emplace_back(1 + i, 6 + i);

		edges.emplace_back(6 + i, 11 + i);
		edges.emplace_back(6 + j, 11 + i);
		edges.emplace_back(1 + i, 11 + i);
		edges.emplace_back(1 + j, 11 + i);
	}

	for (int32_t i = 0; i < 5; i++)
	{
		const int32_t j = (i + 1) % 5;
		const glm::vec3 pos = Vertices[11 + i].Normal;

		const glm::vec3 pos1 = reflect(Vertices[1 + j].Position, pos);
		Vertices.emplace_back(pos1, pos1, glm::vec2(0.0f));

		const glm::vec3 pos2 = reflect(Vertices[1 + i].Position, pos);
		Vertices.emplace_back(pos2, pos2, glm::vec2(0.0f));

		edges.emplace_back(11 + i, 16 + 2 * i);
		edges.emplace_back(11 + i, 17 + 2 * i);
		edges.emplace_back(6 + i, 16 + 2 * i);
		edges.emplace_back(6 + j, 17 + 2 * i);
		edges.emplace_back(16 + 2 * i, 17 + 2 * i);
		edges.emplace_back(16 + std::min(2 * i + 1, 2 * j), 16 + std::max(2 * i + 1, 2 * j));
	}

	std::sort(edges.begin(), edges.end());

	for (int32_t subdiv = 0; subdiv < 3; subdiv++)
	{
		std::vector<std::pair<uint32_t, uint32_t>> oldEdges = edges;
		edges.clear();

		const uint32_t indexOffset = static_cast<uint32_t>(Vertices.size());

		for (int32_t i = 0; i < oldEdges.size(); i++)
		{
			glm::vec3 pos = Vertices[oldEdges[i].first].Normal + Vertices[oldEdges[i].second].Normal;
			pos /= glm::length(pos);

			Vertices.emplace_back(radius * pos, pos, glm::vec2(0.0f));

			edges.emplace_back(oldEdges[i].first, i + indexOffset);
			edges.emplace_back(oldEdges[i].second, i + indexOffset);

			for (int32_t j = 0; j < i; j++)
			{
				if (oldEdges[j].second != oldEdges[i].first)
					continue;

				for (int32_t k = j + 1; k < i; k++)
				{
					if (oldEdges[k].first != oldEdges[j].first)
						break;

					if (oldEdges[k].second != oldEdges[i].second)
						continue;

					edges.emplace_back(j + indexOffset, k + indexOffset);
					edges.emplace_back(j + indexOffset, i + indexOffset);
					edges.emplace_back(k + indexOffset, i + indexOffset);
				}
			}
		}

		std::sort(edges.begin(), edges.end());
	}

	const uint32_t hemisphereVertexCount = static_cast<uint32_t>(Vertices.size());
	const uint32_t hemisphereEdgeCount = static_cast<uint32_t>(edges.size());

	for (uint32_t i = 0; i < hemisphereEdgeCount; i++)
	{
		if (Vertices[edges[i].first].Position.x < 1.0e-4f && Vertices[edges[i].second].Position.x < 1.0e-4f)
			edges.emplace_back(edges[i].first, edges[i].second + hemisphereVertexCount);

		edges.emplace_back(edges[i].first + hemisphereVertexCount, edges[i].second + hemisphereVertexCount);
	}

	for (uint32_t i = 0; i < hemisphereVertexCount; i++)
	{
		if (Vertices[i].Position.x < 1.0e-4f)
			edges.emplace_back(i, i + hemisphereVertexCount);

		Vertices[i].Position.x += halfHeight;
		Vertices.push_back(Vertices[i]);
		Vertices[i + hemisphereVertexCount].Position.x = -Vertices[i].Position.x;
		Vertices[i + hemisphereVertexCount].Normal.x = -Vertices[i].Normal.x;
	}

	std::sort(edges.begin(), edges.end());

	for (int32_t i = 0; i < edges.size(); i++)
		for (int32_t j = i + 1; j < edges.size(); j++)
		{
			if (edges[i].first != edges[j].first)
				continue;

			if (!std::binary_search(edges.begin(), edges.end(), std::make_pair(edges[i].second, edges[j].second)))
				continue;

			Indices.push_back(edges[i].first);
			Indices.push_back(edges[i].second);
			Indices.push_back(edges[j].second);
		}
}

Model::Model(Device& device, const Builder& builder) :
	mDevice(device)
{
	CreateVertexBuffers(builder.Vertices);
	CreateIndexBuffers(builder.Indices);
}

Model::~Model()
{
	vkDestroyBuffer(mDevice.GetDevice(), mVertexBuffer, nullptr);
	vkFreeMemory(mDevice.GetDevice(), mVertexBufferMemory, nullptr);

	if (mIndexCount > 0)
	{
		vkDestroyBuffer(mDevice.GetDevice(), mIndexBuffer, nullptr);
		vkFreeMemory(mDevice.GetDevice(), mIndexBufferMemory, nullptr);
	}
}

std::shared_ptr<Model> Model::CreateFromFile(Device& device, const std::string& path)
{
	Builder builder;
	builder.LoadModel(path);

	return std::make_shared<Model>(device, builder);
}

std::shared_ptr<Model> Model::CreateCube(Device& device, float halfExtentX, float halfExtentY, float halfExtentZ)
{
	static Builder defaultBuilder;
	static bool madeDefault = false;
	if (!madeDefault)
	{
		defaultBuilder.MakeCube(1.0f, 1.0f, 1.0f);
		madeDefault = true;
	}

	Builder builder = defaultBuilder;
	for (Model::Vertex& vertex : builder.Vertices)
		vertex.Position *= glm::vec3(halfExtentX, halfExtentY, halfExtentZ);

	return std::make_shared<Model>(device, builder);
}

std::shared_ptr<Model> Model::CreateSphere(Device& device, float radius)
{
	static Builder defaultBuilder;
	static bool madeDefault = false;
	if (!madeDefault)
	{
		defaultBuilder.MakeSphere(1.0f);
		madeDefault = true;
	}

	Builder builder = defaultBuilder;
	for (Model::Vertex& vertex : builder.Vertices)
		vertex.Position *= radius;

	return std::make_shared<Model>(device, builder);
}

std::shared_ptr<Model> Model::CreateCylinder(Device& device, float radius, float halfHeight)
{
	static Builder defaultBuilder;
	static bool madeDefault = false;
	if (!madeDefault)
	{
		defaultBuilder.MakeCylinder(1.0f, 1.0f);
		madeDefault = true;
	}

	Builder builder = defaultBuilder;
	for (Model::Vertex& vertex : builder.Vertices)
		vertex.Position *= glm::vec3(halfHeight, radius, radius);

	return std::make_shared<Model>(device, builder);
}

std::shared_ptr<Model> Model::CreateCapsule(Device& device, float radius, float halfHeight)
{
	static Builder defaultBuilder;
	static bool madeDefault = false;
	if (!madeDefault)
	{
		defaultBuilder.MakeCapsule(1.0f, 1.0f);
		madeDefault = true;
	}

	Builder builder = defaultBuilder;
	for (Model::Vertex& vertex : builder.Vertices)
	{
		const float hemisphereSign = float(vertex.Position.x > 0.0f) - float(vertex.Position.x < 0.0f);
		vertex.Position.x -= hemisphereSign;
		vertex.Position *= radius;
		vertex.Position.x += hemisphereSign * halfHeight;
	}

	return std::make_shared<Model>(device, builder);
}

void Model::Bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[] = { mVertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	if (mIndexCount > 0)
	{
		vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}
}

void Model::Draw(VkCommandBuffer commandBuffer)
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

void Model::CreateVertexBuffers(const std::vector<Vertex>& vertices)
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

void Model::CreateIndexBuffers(const std::vector<uint32_t>& indices)
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
