#include "Vulkan/Model.h"
#include "Vulkan/Renderer.h"

#define TINYOBJLOADER_IMPLEMENTATION

#include <glm/gtx/hash.hpp>
#include <tiny_obj_loader.h>

namespace std
{
	template<>
	struct hash<Vulkan::Vertex>
	{
		u64 operator()(Vulkan::Vertex const& vertex) const
		{
			u64 seed = 0;
			mage::HashCombine(seed, vertex.Position, vertex.Normal, vertex.UV);
			return seed;
		}
	};
}

namespace Vulkan
{
	mage::Array<vk::VertexInputBindingDescription> Vertex::GetBindingDescriptions()
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

	mage::Array<vk::VertexInputAttributeDescription> Vertex::GetAttributeDescriptions()
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
				.format = vk::Format::eR32G32B32Sfloat,
				.offset = offsetof(Vertex, UV)
			}
		};
	}

	Model::Model(Renderer const& inRenderer, ModelInfo const& inCreateInfo)
	{
		CreateVertexBuffer(inRenderer, inCreateInfo.Vertices);
		CreateIndexBuffer(inRenderer, inCreateInfo.Indices);
	}

	void Model::Bind(vk::CommandBuffer inCommandBuffer) const
	{
		mVertexBuffer.BindVertexBuffer(inCommandBuffer);

		if (mIndexCount > 0)
			mIndexBuffer.BindIndexBuffer(inCommandBuffer);
	}

	void Model::Draw(vk::CommandBuffer inCommandBuffer) const
	{
		if (mIndexCount > 0)
			inCommandBuffer.drawIndexed(mIndexCount, 1, 0, 0, 0);
		else
			inCommandBuffer.draw(mVertexCount, 1, 0, 0);
	}

	ModelInfo Model::LoadFromFile(mage::StringView inPath)
	{
		ModelInfo result;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		bool loadResult = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inPath.GetCString());
		mage_check(loadResult);

		std::unordered_map<Vertex, u32> uniqueVertices;

		for (tinyobj::shape_t const& shape : shapes)
		{
			for (tinyobj::index_t const& index : shape.mesh.indices)
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
					uniqueVertices[vertex] = result.Vertices.GetSize();
					result.Vertices.Add(vertex);
				}

				result.Indices.Add(uniqueVertices[vertex]);
			}
		}

		FixWindingOrders(result);
		return result;
	}

	ModelInfo Model::MakeCube(f32 halfExtentX, f32 halfExtentY, f32 halfExtentZ)
	{
		ModelInfo result;

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

		glm::vec3 halfExtent(halfExtentX, halfExtentY, halfExtentZ);

		for (u32 i = 0; i < 6; ++i)
		{
			for (u32 j = 0; j < 8; ++j)
			{
				glm::vec3 test = positions[j] * normals[i];

				if (test.x < 0.0f)
					continue;

				if (test.y < 0.0f)
					continue;

				if (test.z < 0.0f)
					continue;

				result.Vertices.AddConstruct(halfExtent * positions[j], normals[i], glm::vec2(0.0f));
			}

			for (u32 j = 0; j < 6; ++j)
				result.Indices.Add(4 * i + (j / 3) + (j % 3));
		}

		FixWindingOrders(result);
		return result;
	}

	ModelInfo Model::MakeSphere(f32 radius)
	{
		ModelInfo result;

		constexpr f32 phi = glm::golden_ratio<f32>();
		f32 factor = glm::inversesqrt(1.0f + phi * phi);

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

		for (u32 i = 0; i < 12; ++i)
		{
			glm::vec3 pos = factor * positions[i];

			result.Vertices.AddConstruct(radius * pos, pos, glm::vec2(0.0f));
		}

		mage::Array<std::pair<u32, u32>> edges;

		for (u32 i = 0; i < 12; i += 2)
		{
			edges.AddConstruct(i, i + 1);
		}

		for (u32 i = 0; i < 3; ++i)
			for (u32 j = 0; j < 4; ++j)
				for (u32 k = 0; k < 4; ++k)
				{
					u32 indexA = (4 * i + j) % 12;
					u32 indexB = (4 * i + k + 4) % 12;

					glm::vec3 test = positions[indexA] * positions[indexB];

					if (test.x < 0.0f)
						continue;

					if (test.y < 0.0f)
						continue;

					if (test.z < 0.0f)
						continue;

					if (indexA < indexB)
						edges.AddConstruct(indexA, indexB);
					else
						edges.AddConstruct(indexB, indexA);
				}

		std::sort(edges.begin(), edges.end());

		for (u32 subdiv = 0; subdiv < 3; ++subdiv)
		{
			mage::Array<std::pair<u32, u32>> oldEdges = edges;
			edges.Empty();

			u32 indexOffset = result.Vertices.GetSize();

			for (u32 i = 0; i < oldEdges.GetSize(); ++i)
			{
				glm::vec3 pos = result.Vertices[oldEdges[i].first].Normal + result.Vertices[oldEdges[i].second].Normal;
				pos /= glm::length(pos);

				result.Vertices.AddConstruct(radius * pos, pos, glm::vec2(0.0f));

				edges.AddConstruct(oldEdges[i].first, i + indexOffset);
				edges.AddConstruct(oldEdges[i].second, i + indexOffset);

				for (u32 j = 0; j < i; ++j)
				{
					if (oldEdges[j].second != oldEdges[i].first)
						continue;

					for (u32 k = j + 1; k < i; ++k)
					{
						if (oldEdges[k].first != oldEdges[j].first)
							break;

						if (oldEdges[k].second != oldEdges[i].second)
							continue;

						edges.AddConstruct(j + indexOffset, k + indexOffset);
						edges.AddConstruct(j + indexOffset, i + indexOffset);
						edges.AddConstruct(k + indexOffset, i + indexOffset);
					}
				}
			}

			std::sort(edges.begin(), edges.end());
		}

		for (u32 i = 0; i < edges.GetSize(); ++i)
			for (u32 j = i + 1; j < edges.GetSize(); ++j)
			{
				if (edges[i].first != edges[j].first)
					continue;

				if (!std::binary_search(edges.begin(), edges.end(), std::make_pair(edges[i].second, edges[j].second)))
					continue;

				result.Indices.Add(edges[i].first);
				result.Indices.Add(edges[i].second);
				result.Indices.Add(edges[j].second);
			}

		FixWindingOrders(result);
		return result;
	}

	ModelInfo Model::MakeCylinder(f32 radius, f32 halfHeight)
	{
		ModelInfo result;

		mage::Array<std::pair<u32, u32>> edges;
		
		glm::vec3 halfHeightVec = glm::vec3(halfHeight, 0.0f, 0.0f);
		glm::vec3 normalVec = glm::vec3(1.0f, 0.0f, 0.0f);
		
		for (u32 i = 0; i < 80; ++i)
		{
			f32 c = glm::cos(glm::radians(4.5f * i));
			f32 s = glm::sin(glm::radians(4.5f * i));
			glm::vec3 base = glm::vec3(0.0f, c, s);
		
			result.Vertices.AddConstruct(radius * base + halfHeightVec, base, glm::vec2(0.0f));
			result.Vertices.AddConstruct(radius * base - halfHeightVec, base, glm::vec2(0.0f));
		
			edges.AddConstruct(2 * i, 2 * i + 1);
			edges.AddConstruct(i ? 2 * i - 1 : 0, i ? 2 * i : 159);
		
			edges.AddConstruct(0 + (i ? 2 * i - 2 : 0), 0 + (i ? 2 * i : 158));
			edges.AddConstruct(1 + (i ? 2 * i - 2 : 0), 1 + (i ? 2 * i : 158));
		}
		
		for (u32 i = 0; i < 80; ++i)
		{
			f32 c = glm::cos(glm::radians(4.5f * i));
			f32 s = glm::sin(glm::radians(4.5f * i));
			glm::vec3 base = glm::vec3(0.0f, c, s);
		
			result.Vertices.AddConstruct(radius * base + halfHeightVec, normalVec, glm::vec2(0.0f));
			result.Vertices.AddConstruct(radius * base - halfHeightVec, -normalVec, glm::vec2(0.0f));
		
			edges.AddConstruct(160 + (i ? 2 * i - 2 : 0), 160 + (i ? 2 * i : 158));
			edges.AddConstruct(161 + (i ? 2 * i - 2 : 0), 161 + (i ? 2 * i : 158));
		}
		
		for (u32 i = 0; i < 40; ++i)
		{
			f32 c = glm::cos(glm::radians(9.0f * i));
			f32 s = glm::sin(glm::radians(9.0f * i));
			glm::vec3 base = glm::vec3(0.0f, c, s);
		
			result.Vertices.AddConstruct(0.9f * radius * base + halfHeightVec, normalVec, glm::vec2(0.0f));
			result.Vertices.AddConstruct(0.9f * radius * base - halfHeightVec, -normalVec, glm::vec2(0.0f));
		
			edges.AddConstruct(320 + (i ? 2 * i - 2 : 0), 320 + (i ? 2 * i : 78));
			edges.AddConstruct(160 + (i ? 4 * i - 2 : 158), 320 + 2 * i);
			edges.AddConstruct(160 + 4 * i, 320 + 2 * i);
			edges.AddConstruct(160 + 4 * i + 2, 320 + 2 * i);
		
			edges.AddConstruct(321 + (i ? 2 * i - 2 : 0), 321 + (i ? 2 * i : 78));
			edges.AddConstruct(161 + (i ? 4 * i - 2 : 158), 321 + 2 * i);
			edges.AddConstruct(161 + 4 * i, 321 + 2 * i);
			edges.AddConstruct(161 + 4 * i + 2, 321 + 2 * i);
		}
		
		for (u32 i = 0; i < 20; ++i)
		{
			f32 c = glm::cos(glm::radians(18.0f * i));
			f32 s = glm::sin(glm::radians(18.0f * i));
			glm::vec3 base = glm::vec3(0.0f, c, s);
		
			result.Vertices.AddConstruct(0.75f * radius * base + halfHeightVec, normalVec, glm::vec2(0.0f));
			result.Vertices.AddConstruct(0.75f * radius * base - halfHeightVec, -normalVec, glm::vec2(0.0f));
		
			edges.AddConstruct(400 + (i ? 2 * i - 2 : 0), 400 + (i ? 2 * i : 38));
			edges.AddConstruct(320 + (i ? 4 * i - 2 : 78), 400 + 2 * i);
			edges.AddConstruct(320 + 4 * i, 400 + 2 * i);
			edges.AddConstruct(320 + 4 * i + 2, 400 + 2 * i);
		
			edges.AddConstruct(401 + (i ? 2 * i - 2 : 0), 401 + (i ? 2 * i : 38));
			edges.AddConstruct(321 + (i ? 4 * i - 2 : 78), 401 + 2 * i);
			edges.AddConstruct(321 + 4 * i, 401 + 2 * i);
			edges.AddConstruct(321 + 4 * i + 2, 401 + 2 * i);
		}
		
		for (u32 i = 0; i < 10; ++i)
		{
			f32 c = glm::cos(glm::radians(36.0f * i));
			f32 s = glm::sin(glm::radians(36.0f * i));
			glm::vec3 base = glm::vec3(0.0f, c, s);
		
			result.Vertices.AddConstruct(0.55f * radius * base + halfHeightVec, normalVec, glm::vec2(0.0f));
			result.Vertices.AddConstruct(0.55f * radius * base - halfHeightVec, -normalVec, glm::vec2(0.0f));
		
			edges.AddConstruct(440 + (i ? 2 * i - 2 : 0), 440 + (i ? 2 * i : 18));
			edges.AddConstruct(400 + (i ? 4 * i - 2 : 38), 440 + 2 * i);
			edges.AddConstruct(400 + 4 * i, 440 + 2 * i);
			edges.AddConstruct(400 + 4 * i + 2, 440 + 2 * i);
		
			edges.AddConstruct(441 + (i ? 2 * i - 2 : 0), 441 + (i ? 2 * i : 18));
			edges.AddConstruct(401 + (i ? 4 * i - 2 : 38), 441 + 2 * i);
			edges.AddConstruct(401 + 4 * i, 441 + 2 * i);
			edges.AddConstruct(401 + 4 * i + 2, 441 + 2 * i);
		}
		
		for (u32 i = 0; i < 5; ++i)
		{
			f32 c = glm::cos(glm::radians(72.0f * i));
			f32 s = glm::sin(glm::radians(72.0f * i));
			glm::vec3 base = glm::vec3(0.0f, c, s);
		
			result.Vertices.AddConstruct(0.3f * radius * base + halfHeightVec, normalVec, glm::vec2(0.0f));
			result.Vertices.AddConstruct(0.3f * radius * base - halfHeightVec, -normalVec, glm::vec2(0.0f));
		
			edges.AddConstruct(460 + (i ? 2 * i - 2 : 0), 460 + (i ? 2 * i : 8));
			edges.AddConstruct(440 + (i ? 4 * i - 2 : 18), 460 + 2 * i);
			edges.AddConstruct(440 + 4 * i, 460 + 2 * i);
			edges.AddConstruct(440 + 4 * i + 2, 460 + 2 * i);
			edges.AddConstruct(460 + 2 * i, 470);
		
			edges.AddConstruct(461 + (i ? 2 * i - 2 : 0), 461 + (i ? 2 * i : 8));
			edges.AddConstruct(441 + (i ? 4 * i - 2 : 18), 461 + 2 * i);
			edges.AddConstruct(441 + 4 * i, 461 + 2 * i);
			edges.AddConstruct(441 + 4 * i + 2, 461 + 2 * i);
			edges.AddConstruct(461 + 2 * i, 471);
		}
		
		result.Vertices.AddConstruct(halfHeightVec, normalVec, glm::vec2(0.0f));
		result.Vertices.AddConstruct(-halfHeightVec, -normalVec, glm::vec2(0.0f));
		
		std::sort(edges.begin(), edges.end());
		
		for (u32 i = 0; i < edges.GetSize(); ++i)
			for (u32 j = i + 1; j < edges.GetSize(); ++j)
			{
				if (edges[i].first != edges[j].first)
					continue;
		
				if (!std::binary_search(edges.begin(), edges.end(), std::make_pair(edges[i].second, edges[j].second)))
					continue;
		
				result.Indices.Add(edges[i].first);
				result.Indices.Add(edges[i].second);
				result.Indices.Add(edges[j].second);
			}

		FixWindingOrders(result);
		return result;
	}

	ModelInfo Model::MakeCapsule(f32 radius, f32 halfHeight)
	{
		ModelInfo result;

		constexpr f32 phi = glm::golden_ratio<f32>();
		auto reflect = [](glm::vec3 a, glm::vec3 b) -> glm::vec3 { return 2.0f * glm::dot(a, b) * b - a; };

		mage::Array<std::pair<u32, u32>> edges;

		glm::vec3 tip = glm::vec3(1.0f, 0.0f, 0.0f);
		result.Vertices.AddConstruct(radius * tip, tip, glm::vec2(0.0f));

		glm::vec3 first = glm::vec3(glm::sqrt(1.0f - 1.0f / (glm::sqrt(5.0f) * phi)), 1.0f / glm::sqrt(glm::sqrt(5.0f) * phi), 0.0f);
		result.Vertices.AddConstruct(radius * first, first, glm::vec2(0.0f));

		for (u32 i = 1; i < 5; ++i)
		{
			mage::Rotor rotor = mage::Rotor::FromAxisAndAngle(tip, glm::radians(72.0f * i));
			glm::vec3 rotated = rotor.Rotate(first);
			result.Vertices.AddConstruct(radius * rotated, rotated, glm::vec2(0.0f));
		}

		glm::vec3 second = reflect(tip, first);
		result.Vertices.AddConstruct(radius * second, second, glm::vec2(0.0f));

		for (u32 i = 1; i < 5; ++i)
		{
			mage::Rotor rotor = mage::Rotor::FromAxisAndAngle(tip, glm::radians(72.0f * i));
			glm::vec3 rotated = rotor.Rotate(second);
			result.Vertices.AddConstruct(radius * rotated, rotated, glm::vec2(0.0f));
		}

		for (u32 i = 0; i < 5; ++i)
		{
			u32 j = (i + 1) % 5;
			glm::vec3 pos = result.Vertices[6 + i].Position + result.Vertices[6 + j].Position;
			pos /= glm::length(pos);

			result.Vertices.AddConstruct(radius * pos, pos, glm::vec2(0.0f));

			edges.AddConstruct(0, 1 + i);
			edges.AddConstruct(1 + std::min(i, j), 1 + std::max(i, j));
			edges.AddConstruct(1 + i, 6 + i);

			edges.AddConstruct(6 + i, 11 + i);
			edges.AddConstruct(6 + j, 11 + i);
			edges.AddConstruct(1 + i, 11 + i);
			edges.AddConstruct(1 + j, 11 + i);
		}

		for (u32 i = 0; i < 5; ++i)
		{
			u32 j = (i + 1) % 5;
			glm::vec3 pos = result.Vertices[11 + i].Normal;

			glm::vec3 pos1 = reflect(result.Vertices[1 + j].Position, pos);
			result.Vertices.AddConstruct(pos1, pos1, glm::vec2(0.0f));

			glm::vec3 pos2 = reflect(result.Vertices[1 + i].Position, pos);
			result.Vertices.AddConstruct(pos2, pos2, glm::vec2(0.0f));

			edges.AddConstruct(11 + i, 16 + 2 * i);
			edges.AddConstruct(11 + i, 17 + 2 * i);
			edges.AddConstruct(6 + i, 16 + 2 * i);
			edges.AddConstruct(6 + j, 17 + 2 * i);
			edges.AddConstruct(16 + 2 * i, 17 + 2 * i);
			edges.AddConstruct(16 + std::min(2 * i + 1, 2 * j), 16 + std::max(2 * i + 1, 2 * j));
		}

		std::sort(edges.begin(), edges.end());

		for (u32 subdiv = 0; subdiv < 3; ++subdiv)
		{
			mage::Array<std::pair<u32, u32>> oldEdges = edges;
			edges.Empty();

			u32 indexOffset = result.Vertices.GetSize();

			for (u32 i = 0; i < oldEdges.GetSize(); ++i)
			{
				glm::vec3 pos = result.Vertices[oldEdges[i].first].Normal + result.Vertices[oldEdges[i].second].Normal;
				pos /= glm::length(pos);

				result.Vertices.AddConstruct(radius * pos, pos, glm::vec2(0.0f));

				edges.AddConstruct(oldEdges[i].first, i + indexOffset);
				edges.AddConstruct(oldEdges[i].second, i + indexOffset);

				for (u32 j = 0; j < i; ++j)
				{
					if (oldEdges[j].second != oldEdges[i].first)
						continue;

					for (u32 k = j + 1; k < i; ++k)
					{
						if (oldEdges[k].first != oldEdges[j].first)
							break;

						if (oldEdges[k].second != oldEdges[i].second)
							continue;

						edges.AddConstruct(j + indexOffset, k + indexOffset);
						edges.AddConstruct(j + indexOffset, i + indexOffset);
						edges.AddConstruct(k + indexOffset, i + indexOffset);
					}
				}
			}

			std::sort(edges.begin(), edges.end());
		}

		u32 hemisphereVertexCount = result.Vertices.GetSize();
		u32 hemisphereEdgeCount = edges.GetSize();

		for (u32 i = 0; i < hemisphereEdgeCount; ++i)
		{
			if (result.Vertices[edges[i].first].Position.x < 1.0e-4f && result.Vertices[edges[i].second].Position.x < 1.0e-4f)
				edges.AddConstruct(edges[i].first, edges[i].second + hemisphereVertexCount);

			edges.AddConstruct(edges[i].first + hemisphereVertexCount, edges[i].second + hemisphereVertexCount);
		}

		for (u32 i = 0; i < hemisphereVertexCount; ++i)
		{
			if (result.Vertices[i].Position.x < 1.0e-4f)
				edges.AddConstruct(i, i + hemisphereVertexCount);

			result.Vertices[i].Position.x += halfHeight;
			result.Vertices.Add(result.Vertices[i]);
			result.Vertices[i + hemisphereVertexCount].Position.x = -result.Vertices[i].Position.x;
			result.Vertices[i + hemisphereVertexCount].Normal.x = -result.Vertices[i].Normal.x;
		}

		std::sort(edges.begin(), edges.end());

		for (u32 i = 0; i < edges.GetSize(); ++i)
			for (u32 j = i + 1; j < edges.GetSize(); ++j)
			{
				if (edges[i].first != edges[j].first)
					continue;

				if (!std::binary_search(edges.begin(), edges.end(), std::make_pair(edges[i].second, edges[j].second)))
					continue;

				result.Indices.Add(edges[i].first);
				result.Indices.Add(edges[i].second);
				result.Indices.Add(edges[j].second);
			}

		FixWindingOrders(result);
		return result;
	}

	void Model::FixWindingOrders(ModelInfo& inModelInfo)
	{
		if (inModelInfo.Indices.GetSize() > 0)
		{
			for (u32 i = 0; i < inModelInfo.Indices.GetSize(); i += 3)
			{
				Vertex& a = inModelInfo.Vertices[inModelInfo.Indices[i]];
				Vertex& b = inModelInfo.Vertices[inModelInfo.Indices[i + 1]];
				Vertex& c = inModelInfo.Vertices[inModelInfo.Indices[i + 2]];

				if (glm::dot(a.Normal, glm::cross(b.Position - a.Position, c.Position - a.Position)) > 0.0f)
					std::swap(inModelInfo.Indices[i + 1], inModelInfo.Indices[i + 2]);
			}
		}
		else
		{
			for (u32 i = 0; i < inModelInfo.Vertices.GetSize(); i += 3)
			{
				Vertex& a = inModelInfo.Vertices[i];
				Vertex& b = inModelInfo.Vertices[i + 1];
				Vertex& c = inModelInfo.Vertices[i + 2];

				if (glm::dot(a.Normal, glm::cross(b.Position - a.Position, c.Position - a.Position)) > 0.0f)
					std::swap(inModelInfo.Vertices[i + 1], inModelInfo.Vertices[i + 2]);
			}
		}
	}

	void Model::CreateVertexBuffer(Renderer const& inRenderer, mage::Array<Vertex> const& inVertices)
	{
		mVertexCount = inVertices.GetSize();
		mage_ensure(mVertexCount >= 3);

		vk::DeviceSize dataSize = mVertexCount * sizeof(Vertex);

		Vulkan::BufferCreateInfo stagingBufferCreateInfo
		{
			.Size = dataSize,
			.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		};

		Vulkan::Buffer stagingBuffer = inRenderer.CreateBuffer(stagingBufferCreateInfo);

		stagingBuffer.Map();
		stagingBuffer.Write(inVertices.GetData(), dataSize);

		Vulkan::BufferCreateInfo vertexBufferCreateInfo
		{
			.Size = dataSize,
			.UsageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
		};

		mVertexBuffer = inRenderer.CreateBuffer(vertexBufferCreateInfo);

		inRenderer.SubmitSingleTimeCommands([this, &inRenderer, &stagingBuffer, dataSize](vk::CommandBuffer inCommandBuffer)
			{
				inRenderer.CopyBuffer(inCommandBuffer, stagingBuffer, mVertexBuffer, dataSize);
			});
	}

	void Model::CreateIndexBuffer(Renderer const& inRenderer, mage::Array<u32> const& inIndices)
	{
		mIndexCount = inIndices.GetSize();
		if (mIndexCount == 0)
			return;

		mage_ensure(mIndexCount >= 3);

		vk::DeviceSize dataSize = mIndexCount * sizeof(u32);

		Vulkan::BufferCreateInfo stagingBufferCreateInfo
		{
			.Size = dataSize,
			.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		};

		Vulkan::Buffer stagingBuffer = inRenderer.CreateBuffer(stagingBufferCreateInfo);

		stagingBuffer.Map();
		stagingBuffer.Write(inIndices.GetData(), dataSize);

		Vulkan::BufferCreateInfo indexBufferCreateInfo
		{
			.Size = dataSize,
			.UsageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
		};

		mIndexBuffer = inRenderer.CreateBuffer(indexBufferCreateInfo);

		inRenderer.SubmitSingleTimeCommands([this, &inRenderer, &stagingBuffer, dataSize](vk::CommandBuffer inCommandBuffer)
			{
				inRenderer.CopyBuffer(inCommandBuffer, stagingBuffer, mIndexBuffer, dataSize);
			});
	}
}
