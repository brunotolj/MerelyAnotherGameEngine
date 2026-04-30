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

	Model::Model(Renderer const& inRenderer, ModelCreateInfo const& inCreateInfo)
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

	ModelCreateInfo Model::LoadFromFile(mage::StringView inPath)
	{
		ModelCreateInfo result;

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

	ModelCreateInfo Model::MakeCube(glm::vec3 inHalfExtent)
	{
		ModelCreateInfo result;

		glm::vec3 positions[8]
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

		glm::vec3 normals[6]
		{
			{ -1.0f,  0.0f,  0.0f },
			{  1.0f,  0.0f,  0.0f },
			{  0.0f, -1.0f,  0.0f },
			{  0.0f,  1.0f,  0.0f },
			{  0.0f,  0.0f, -1.0f },
			{  0.0f,  0.0f,  1.0f }
		};

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

				result.Vertices.AddConstruct(inHalfExtent * positions[j], normals[i], glm::vec2(0.0f));
			}

			for (u32 j = 0; j < 6; ++j)
				result.Indices.Add(4 * i + (j / 3) + (j % 3));
		}

		FixWindingOrders(result);
		return result;
	}

	ModelCreateInfo Model::MakeSphere(f32 inRadius)
	{
		ModelCreateInfo result;

		mage::Array<std::pair<u32, u32>> edges;
		auto AddEdge = [&edges](u32 a, u32 b) { (a < b) ? edges.AddConstruct(a, b) : edges.AddConstruct(b, a); };
		auto SortEdges = [&edges]() { std::sort(edges.begin(), edges.end()); };

		f32 phi = glm::golden_ratio<f32>();
		f32 factor = glm::inversesqrt(1.0f + phi * phi);

		glm::vec3 baseIcosphere[12]
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
			glm::vec3 pos = factor * baseIcosphere[i];
			result.Vertices.AddConstruct(inRadius * pos, pos, glm::vec2(0.0f));

			for (u32 j = i + 1; j < 12; ++j)
			{
				f32 threshold = phi - 0.01f;
				if (glm::dot(baseIcosphere[i], baseIcosphere[j]) > threshold)
					AddEdge(i, j);
			}
		}

		for (u32 subdiv = 0; subdiv < 3; ++subdiv)
		{
			SortEdges();

			mage::Array<std::pair<u32, u32>> oldEdges = edges;
			edges.Empty();

			u32 indexOffset = result.Vertices.GetSize();

			for (u32 i = 0; i < oldEdges.GetSize(); ++i)
			{
				glm::vec3 pos = result.Vertices[oldEdges[i].first].Normal + result.Vertices[oldEdges[i].second].Normal;
				pos /= glm::length(pos);

				result.Vertices.AddConstruct(inRadius * pos, pos, glm::vec2(0.0f));

				AddEdge(oldEdges[i].first, i + indexOffset);
				AddEdge(oldEdges[i].second, i + indexOffset);

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

						AddEdge(j + indexOffset, k + indexOffset);
						AddEdge(j + indexOffset, i + indexOffset);
						AddEdge(k + indexOffset, i + indexOffset);
					}
				}
			}
		}

		SortEdges();
		PopulateIndices(result, edges);
		
		FixWindingOrders(result);
		return result;
	}

	ModelCreateInfo Model::MakeCylinder(f32 inRadius, f32 inHalfHeight)
	{
		ModelCreateInfo result;

		mage::Array<std::pair<u32, u32>> edges;
		auto AddEdge = [&edges](u32 a, u32 b) { (a < b) ? edges.AddConstruct(a, b) : edges.AddConstruct(b, a); };
		auto SortEdges = [&edges]() { std::sort(edges.begin(), edges.end()); };

		using Subset = std::pair<u32, u32>;
		
		auto CreateRing = [&result, &edges, &AddEdge](u32 count, glm::vec3 base, glm::vec3 normal, glm::vec3 axis) -> Subset
			{
				u32 indexOffset = result.Vertices.GetSize();

				for (u32 i = 0; i < count; ++i)
				{
					mage::Rotor rotor = mage::Rotor::FromAxisAndAngle(axis, glm::radians(i * (360.0f / count)));
					glm::vec3 rotatedBase = rotor.Rotate(base);
					glm::vec3 rotatedNormal = rotor.Rotate(normal);

					result.Vertices.AddConstruct(rotatedBase, rotatedNormal, glm::vec2(0.0f));
					AddEdge(indexOffset + i, indexOffset + (i + 1) % count);
				}

				return { indexOffset, count };
			};

		auto ConnectRings = [&edges, &AddEdge](Subset ringA, Subset ringB)
			{
				u32 indexA = 0;
				u32 indexB = 0;
				i32 distanceA = 0;
				i32 distanceB = 0;

				while (indexA < ringA.second && indexB < ringB.second)
				{
					if (glm::abs(distanceA + ringB.second - distanceB) <= glm::abs(distanceB + ringA.second - distanceA))
					{
						distanceA += ringB.second;
						indexA++;
					}
					else
					{
						distanceB += ringA.second;
						indexB++;
					}

					AddEdge(ringA.first + indexA % ringA.second, ringB.first + indexB % ringB.second);
				}

				while (indexA < ringA.second)
				{
					indexA++;
					AddEdge(ringA.first + indexA % ringA.second, ringB.first + indexB % ringB.second);
				}

				while (indexB < ringB.second)
				{
					indexB++;
					AddEdge(ringA.first + indexA % ringA.second, ringB.first + indexB % ringB.second);
				}
			};

		mage::Array<std::pair<u32, f32>> ringData
		{
			{ 80, 1.0f },
			{ 40, 0.9f },
			{ 20, 0.75f },
			{ 10, 0.55f },
			{ 5, 0.3f },
			{ 1, 0.0f }
		};

		glm::vec3 forward = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 right = glm::vec3(0.0f, 1.0f, 0.0f);

		mage::Array<Subset> sideRings{ CreateRing(ringData[0].first, ringData[0].second * inRadius * right + inHalfHeight * forward, right, forward) };

		u32 sideDivCount = 1 + u32(0.2f * ringData[0].first * inHalfHeight / inRadius);
		for (u32 i = 1; i <= sideDivCount; ++i)
		{
			f32 ringHeight = inHalfHeight * (1.0f - 2.0f * i / sideDivCount);

			sideRings.Add(CreateRing(ringData[0].first, ringData[0].second * inRadius * right + ringHeight * forward, right, forward));
			ConnectRings(sideRings[i - 1], sideRings[i]);
		}

		mage::Array<Subset> topRings;
		mage::Array<Subset> bottomRings;

		for (u32 i = 0; i < ringData.GetSize(); ++i)
		{
			topRings.Add(CreateRing(ringData[i].first, ringData[i].second * inRadius * right + inHalfHeight * forward, forward, forward));
			bottomRings.Add(CreateRing(ringData[i].first, ringData[i].second * inRadius * right - inHalfHeight * forward, -forward, forward));
			if (i > 0)
			{
				ConnectRings(topRings[i - 1], topRings[i]);
				ConnectRings(bottomRings[i - 1], bottomRings[i]);
			}
		}

		std::sort(edges.begin(), edges.end());
		PopulateIndices(result, edges);

		FixWindingOrders(result);
		return result;
	}

	ModelCreateInfo Model::MakeCapsule(f32 inRadius, f32 inHalfHeight)
	{
		ModelCreateInfo result;

		mage::Array<std::pair<u32, u32>> edges;
		auto AddEdge = [&edges](u32 a, u32 b) { (a < b) ? edges.AddConstruct(a, b) : edges.AddConstruct(b, a); };
		auto SortEdges = [&edges]() { std::sort(edges.begin(), edges.end()); };
		auto Reflect = [](glm::vec3 a, glm::vec3 b) -> glm::vec3 { return 2.0f * glm::dot(a, b) * b - a; };

		using Subset = std::pair<u32, u32>;

		auto CreateRing = [&result, &edges, &AddEdge](u32 count, glm::vec3 base, glm::vec3 normal, glm::vec3 axis, bool connect) -> Subset
			{
				u32 indexOffset = result.Vertices.GetSize();

				for (u32 i = 0; i < count; ++i)
				{
					mage::Rotor rotor = mage::Rotor::FromAxisAndAngle(axis, glm::radians(i * (360.0f / count)));
					glm::vec3 rotatedBase = rotor.Rotate(base);
					glm::vec3 rotatedNormal = rotor.Rotate(normal);

					result.Vertices.AddConstruct(rotatedBase, rotatedNormal, glm::vec2(0.0f));

					if (connect)
						AddEdge(indexOffset + i, indexOffset + (i + 1) % count);
				}

				return { indexOffset, count };
			};

		auto ConnectRings = [&edges, &AddEdge](Subset ringA, Subset ringB)
			{
				u32 indexA = 0;
				u32 indexB = 0;
				i32 distanceA = 0;
				i32 distanceB = 0;

				while (indexA < ringA.second && indexB < ringB.second)
				{
					if (glm::abs(distanceA + ringB.second - distanceB) <= glm::abs(distanceB + ringA.second - distanceA))
					{
						distanceA += ringB.second;
						indexA++;
					}
					else
					{
						distanceB += ringA.second;
						indexB++;
					}

					AddEdge(ringA.first + indexA % ringA.second, ringB.first + indexB % ringB.second);
				}

				while (indexA < ringA.second)
				{
					indexA++;
					AddEdge(ringA.first + indexA % ringA.second, ringB.first + indexB % ringB.second);
				}

				while (indexB < ringB.second)
				{
					indexB++;
					AddEdge(ringA.first + indexA % ringA.second, ringB.first + indexB % ringB.second);
				}
			};

		f32 phi = glm::golden_ratio<f32>();
		glm::vec3 forward = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 right = glm::vec3(0.0f, 1.0f, 0.0f);

		Subset ring0 = CreateRing(1, inRadius * forward, forward, forward, false);

		glm::vec3 base1 = glm::vec3(glm::sqrt(1.0f - 1.0f / (glm::sqrt(5.0f) * phi)), 1.0f / glm::sqrt(glm::sqrt(5.0f) * phi), 0.0f);
		Subset ring1 = CreateRing(5, inRadius * base1, base1, forward, true);
		
		glm::vec3 base2 = Reflect(forward, base1);
		Subset ring2 = CreateRing(5, inRadius * base2, base2, forward, false);
		
		glm::vec3 base3 = glm::normalize(result.Vertices[6].Position + result.Vertices[7].Position);
		Subset ring3 = CreateRing(5, inRadius * base3, base3, forward, false);

		glm::vec3 base4 = mage::Rotor::FromAxisAndAngle(forward, glm::radians(18.0f)).Rotate(right);
		Subset ring4 = CreateRing(10, inRadius * base4, base4, forward, true);

		ConnectRings(ring0, ring1);
		ConnectRings(ring1, ring3);
		ConnectRings(ring2, ring3);

		for (u32 i = 0; i < 5; ++i)
		{
			edges.AddConstruct(ring1.first + i, ring2.first + i);
			edges.AddConstruct(ring1.first + i, ring2.first + i);

			edges.AddConstruct(ring2.first + i, ring4.first + 2 * i);
			edges.AddConstruct(ring2.first + (i + 1) % 5, ring4.first + 2 * i + 1);
			edges.AddConstruct(ring3.first + i, ring4.first + 2 * i);
			edges.AddConstruct(ring3.first + i, ring4.first + 2 * i + 1);
		}

		for (u32 subdiv = 0; subdiv < 3; ++subdiv)
		{
			std::sort(edges.begin(), edges.end());

			mage::Array<std::pair<u32, u32>> oldEdges = edges;
			edges.Empty();

			u32 indexOffset = result.Vertices.GetSize();

			for (u32 i = 0; i < oldEdges.GetSize(); ++i)
			{
				glm::vec3 pos = result.Vertices[oldEdges[i].first].Normal + result.Vertices[oldEdges[i].second].Normal;
				pos /= glm::length(pos);

				result.Vertices.AddConstruct(inRadius * pos, pos, glm::vec2(0.0f));

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
		}

		std::sort(edges.begin(), edges.end());

		u32 hemisphereVertexCount = result.Vertices.GetSize();
		u32 hemisphereEdgeCount = edges.GetSize();

		for (u32 i = 0; i < hemisphereEdgeCount; ++i)
			edges.AddConstruct(edges[i].first + hemisphereVertexCount, edges[i].second + hemisphereVertexCount);

		for (u32 i = 0; i < hemisphereVertexCount; ++i)
		{
			if (result.Vertices[i].Position.x < 1.0e-4f)
				edges.AddConstruct(i, i + hemisphereVertexCount);

			result.Vertices[i].Position.x += inHalfHeight;
			result.Vertices.Add(result.Vertices[i]);
			result.Vertices[i + hemisphereVertexCount].Position.x = -result.Vertices[i].Position.x;
			result.Vertices[i + hemisphereVertexCount].Normal.x = -result.Vertices[i].Normal.x;
		}

		u32 ringSize = 80;

		mage::Array<Subset> sideRings{ CreateRing(ringSize, inRadius * right + inHalfHeight * forward, right, forward, true) };
		
		u32 sideDivCount = 1 + u32(0.2f * ringSize * inHalfHeight / inRadius);
		for (u32 i = 1; i <= sideDivCount; ++i)
		{
			f32 ringHeight = inHalfHeight * (1.0f - 2.0f * i / sideDivCount);
		
			sideRings.Add(CreateRing(ringSize, inRadius * right + ringHeight * forward, right, forward, true));
			ConnectRings(sideRings[i - 1], sideRings[i]);
		}

		std::sort(edges.begin(), edges.end());
		PopulateIndices(result, edges);

		FixWindingOrders(result);
		return result;
	}

	ModelCreateInfo Model::MakeCone(f32 inRadius, f32 inHeight)
	{
		ModelCreateInfo result;

		mage::Array<std::pair<u32, u32>> edges;
		auto AddEdge = [&edges](u32 a, u32 b) { (a < b) ? edges.AddConstruct(a, b) : edges.AddConstruct(b, a); };
		auto SortEdges = [&edges]() { std::sort(edges.begin(), edges.end()); };

		using Subset = std::pair<u32, u32>;

		auto CreateRing = [&result, &edges, &AddEdge](u32 count, glm::vec3 base, glm::vec3 normal, glm::vec3 axis) -> Subset
			{
				u32 indexOffset = result.Vertices.GetSize();

				for (u32 i = 0; i < count; ++i)
				{
					mage::Rotor rotor = mage::Rotor::FromAxisAndAngle(axis, glm::radians(i * (360.0f / count)));
					glm::vec3 rotatedBase = rotor.Rotate(base);
					glm::vec3 rotatedNormal = rotor.Rotate(normal);

					result.Vertices.AddConstruct(rotatedBase, rotatedNormal, glm::vec2(0.0f));
					AddEdge(indexOffset + i, indexOffset + (i + 1) % count);
				}

				return { indexOffset, count };
			};

		auto ConnectRings = [&edges, &AddEdge](Subset ringA, Subset ringB)
			{
				u32 indexA = 0;
				u32 indexB = 0;
				i32 distanceA = 0;
				i32 distanceB = 0;

				while (indexA < ringA.second && indexB < ringB.second)
				{
					if (glm::abs(distanceA + ringB.second - distanceB) <= glm::abs(distanceB + ringA.second - distanceA))
					{
						distanceA += ringB.second;
						indexA++;
					}
					else
					{
						distanceB += ringA.second;
						indexB++;
					}

					AddEdge(ringA.first + indexA % ringA.second, ringB.first + indexB % ringB.second);
				}

				while (indexA < ringA.second)
				{
					indexA++;
					AddEdge(ringA.first + indexA % ringA.second, ringB.first + indexB % ringB.second);
				}

				while (indexB < ringB.second)
				{
					indexB++;
					AddEdge(ringA.first + indexA % ringA.second, ringB.first + indexB % ringB.second);
				}
			};

		mage::Array<std::pair<u32, f32>> ringData
		{
			{ 80, 1.0f },
			{ 40, 0.9f },
			{ 20, 0.75f },
			{ 10, 0.55f },
			{ 5, 0.3f },
			{ 1, 0.0f }
		};

		glm::vec3 forward = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 right = glm::vec3(0.0f, 1.0f, 0.0f);

		glm::vec3 normal = glm::normalize(forward + inHeight * right);

		mage::Array<Subset> rings{ CreateRing(80, inRadius * right, normal, forward) };

		u32 divCount = 1 + u32(0.5f * ringData[0].first * inHeight / inRadius);
		for (u32 i = 1; i <= divCount; ++i)
		{
			f32 ratio = glm::pow(f32(divCount - i) / divCount, 3.0f);
			u32 shift = 4 - std::min(4u, divCount - i);

			rings.Add(CreateRing(ringData[0].first >> shift, ratio * inRadius * right + (1.0f - ratio) * inHeight * forward, normal, forward));
			ConnectRings(rings[i - 1], rings[i]);
		}

		mage::Array<Subset> bottomRings;
		for (u32 i = 0; i < ringData.GetSize(); ++i)
		{
			bottomRings.Add(CreateRing(ringData[i].first, ringData[i].second * inRadius * right, -forward, forward));

			if (i > 0)
				ConnectRings(bottomRings[i - 1], bottomRings[i]);
		}

		std::sort(edges.begin(), edges.end());
		PopulateIndices(result, edges);

		FixWindingOrders(result);
		return result;
	}

	void Model::CreateVertexBuffer(Renderer const& inRenderer, mage::Array<Vertex> const& inVertices)
	{
		mVertexCount = inVertices.GetSize();
		mage_ensure(mVertexCount >= 3);

		vk::DeviceSize dataSize = mVertexCount * sizeof(Vertex);

		BufferCreateInfo stagingBufferCreateInfo
		{
			.Size = dataSize,
			.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		};

		Buffer stagingBuffer = inRenderer.CreateBuffer(stagingBufferCreateInfo);

		stagingBuffer.Map();
		stagingBuffer.Write(inVertices.GetData(), dataSize);

		BufferCreateInfo vertexBufferCreateInfo
		{
			.Size = dataSize,
			.UsageFlags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
		};

		mVertexBuffer = inRenderer.CreateBuffer(vertexBufferCreateInfo);

		inRenderer.SubmitSingleTimeCommands([this, &stagingBuffer](vk::CommandBuffer inCommandBuffer)
			{
				mVertexBuffer.CopyFromBuffer(inCommandBuffer, stagingBuffer);
			});
	}

	void Model::CreateIndexBuffer(Renderer const& inRenderer, mage::Array<u32> const& inIndices)
	{
		mIndexCount = inIndices.GetSize();
		if (mIndexCount == 0)
			return;

		mage_ensure(mIndexCount >= 3);

		vk::DeviceSize dataSize = mIndexCount * sizeof(u32);

		BufferCreateInfo stagingBufferCreateInfo
		{
			.Size = dataSize,
			.UsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		};

		Buffer stagingBuffer = inRenderer.CreateBuffer(stagingBufferCreateInfo);

		stagingBuffer.Map();
		stagingBuffer.Write(inIndices.GetData(), dataSize);

		BufferCreateInfo indexBufferCreateInfo
		{
			.Size = dataSize,
			.UsageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			.MemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
		};

		mIndexBuffer = inRenderer.CreateBuffer(indexBufferCreateInfo);

		inRenderer.SubmitSingleTimeCommands([this, &stagingBuffer](vk::CommandBuffer inCommandBuffer)
			{
				mIndexBuffer.CopyFromBuffer(inCommandBuffer, stagingBuffer);
			});
	}

	void Model::PopulateIndices(ModelCreateInfo& inModelInfo, mage::Array<std::pair<u32, u32>> const& inSortedEdges)
	{
		for (u32 i = 0; i < inSortedEdges.GetSize(); ++i)
			for (u32 j = i + 1; j < inSortedEdges.GetSize(); ++j)
			{
				if (inSortedEdges[i].first != inSortedEdges[j].first)
					continue;

				if (!std::binary_search(inSortedEdges.begin(), inSortedEdges.end(), std::make_pair(inSortedEdges[i].second, inSortedEdges[j].second)))
					continue;

				inModelInfo.Indices.Add(inSortedEdges[i].first);
				inModelInfo.Indices.Add(inSortedEdges[i].second);
				inModelInfo.Indices.Add(inSortedEdges[j].second);
			}
	}

	void Model::FixWindingOrders(ModelCreateInfo& inModelInfo)
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
}
