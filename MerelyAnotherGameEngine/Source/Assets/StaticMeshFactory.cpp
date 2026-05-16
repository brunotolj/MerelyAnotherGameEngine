#include "Assets/StaticMeshFactory.h"

#define TINYOBJLOADER_IMPLEMENTATION

#include <glm/gtx/hash.hpp>
#include <tiny_obj_loader.h>

namespace std
{
	template<>
	struct hash<StaticMesh::Vertex>
	{
		u64 operator()(StaticMesh::Vertex const& inValue) const
		{
			u64 seed = 0;
			mage::HashCombine(seed, inValue.Position, inValue.Normal, inValue.TextureCoords);
			return seed;
		}
	};
}

AssetHandle<StaticMesh> Factory<StaticMesh>::FromFile(mage::StringView inPath, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager)
{
	StaticMesh* result = new StaticMesh();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	bool loadResult = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inPath.GetCString());
	mage_check(loadResult);

	std::unordered_map<StaticMesh::Vertex, u32> uniqueVertices;
	mage::Array<u32> indices;

	for (tinyobj::shape_t const& shape : shapes)
	{
		for (tinyobj::index_t const& index : shape.mesh.indices)
		{
			StaticMesh::Vertex vertex{};

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
				vertex.TextureCoords =
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = result->mVertices.GetSize();
				result->mVertices.Add(vertex);
			}

			indices.Add(uniqueVertices[vertex]);
		}
	}

	for (u32 i = 2; i < indices.GetSize(); i += 3)
	{
		StaticMesh::Vertex& a = result->mVertices[indices[i]];
		StaticMesh::Vertex& b = result->mVertices[indices[i + 1]];
		StaticMesh::Vertex& c = result->mVertices[indices[i + 2]];

		if (glm::dot(a.Normal, glm::cross(c.Position - a.Position, b.Position - a.Position)) >= 0.0f)
			result->mFaces.AddConstruct(indices[i - 2], indices[i - 1], indices[i]);
		else
			result->mFaces.AddConstruct(indices[i - 2], indices[i], indices[i - 2]);
	}

	result->CreateVertexBuffer(inRenderer);
	result->CreateIndexBuffer(inRenderer);

	return inAssetManager.Register(result);
}

AssetHandle<StaticMesh> Factory<StaticMesh>::MakeBox(glm::vec3 inHalfExtent, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager)
{
	StaticMesh* result = new StaticMesh();

	glm::vec3 x = { inHalfExtent.x, 0.0f, 0.0f };
	glm::vec3 y = { 0.0f, inHalfExtent.y, 0.0f };
	glm::vec3 z = { 0.0f, 0.0f, inHalfExtent.z };

	AddFlatSurface(*result, { x, {} }, { inHalfExtent.y, inHalfExtent.z }, { 66.0f / 256.0f, 66.0f / 256.0f }, { 128.0f / 256.0f, 128.0f / 256.0f });
	AddFlatSurface(*result, { -x, mage::Rotor(z, glm::radians(180.0f)) }, { inHalfExtent.y, inHalfExtent.z }, { 190.0f / 256.0f, 66.0f / 256.0f }, { 252.0f / 256.0f, 128.0f / 256.0f });
	AddFlatSurface(*result, { -y, mage::Rotor(z, glm::radians(90.0f)) }, { inHalfExtent.x, inHalfExtent.z }, { 4.0f / 256.0f, 66.0f / 256.0f }, { 66.0f / 256.0f, 128.0f / 256.0f });
	AddFlatSurface(*result, { y, mage::Rotor(z, glm::radians(-90.0f)) }, { inHalfExtent.x, inHalfExtent.z }, { 128.0f / 256.0f, 66.0f / 256.0f }, { 190.0f / 256.0f, 128.0f / 256.0f });
	AddFlatSurface(*result, { z, mage::Rotor(y, glm::radians(90.0f)) }, { inHalfExtent.y, inHalfExtent.x }, { 66.0f / 256.0f, 4.0f / 256.0f }, { 128.0f / 256.0f, 66.0f / 256.0f });
	AddFlatSurface(*result, { -z, mage::Rotor(y, glm::radians(-90.0f)) }, { inHalfExtent.y, inHalfExtent.x }, { 66.0f / 256.0f, 128.0f / 256.0f }, { 128.0f / 256.0f, 190.0f / 256.0f });

	result->CreateVertexBuffer(inRenderer);
	result->CreateIndexBuffer(inRenderer);

	return inAssetManager.Register(result);
}

AssetHandle<StaticMesh> Factory<StaticMesh>::MakeBall(f32 inRadius, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager)
{
	StaticMesh* result = new StaticMesh();

	AddHemisphere(*result, {}, inRadius, glm::vec2(75.0f / 256.0f), 71.0f / 256.0f, 3);
	AddInvertedCopy(*result, { 53.0f / 128.0f, 1.0f });

	result->CreateVertexBuffer(inRenderer);
	result->CreateIndexBuffer(inRenderer);

	return inAssetManager.Register(result);
}

AssetHandle<StaticMesh> Factory<StaticMesh>::MakeCylinder(f32 inRadius, f32 inHalfHeight, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager)
{
	StaticMesh* result = new StaticMesh();

	AddCircle(*result, { glm::vec3(inHalfHeight, 0.0f, 0.0f), {} }, inRadius, glm::vec2(65.0f / 256.0f), 61.0f / 256.0f, 4);
	AddInvertedCopy(*result, { 126.0f / 256.0f, 130.0f / 256.0f });
	AddCylindricSurface(*result, {}, inRadius, inHalfHeight, glm::vec2(1.0f / 64.0f, 33.0f / 64.0f), glm::vec2(63.0f / 64.0f), 48);

	result->CreateVertexBuffer(inRenderer);
	result->CreateIndexBuffer(inRenderer);

	return inAssetManager.Register(result);
}

AssetHandle<StaticMesh> Factory<StaticMesh>::MakeCapsule(f32 inRadius, f32 inHalfHeight, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager)
{
	StaticMesh* result = new StaticMesh();

	AddHemisphere(*result, { glm::vec3(inHalfHeight, 0.0f, 0.0f), {} }, inRadius, glm::vec2(65.0f / 256.0f), 61.0f / 256.0f, 3);
	AddInvertedCopy(*result, { 126.0f / 256.0f, 130.0f / 256.0f });
	AddCylindricSurface(*result, {}, inRadius, inHalfHeight, glm::vec2(1.0f / 64.0f, 33.0f / 64.0f), glm::vec2(63.0f / 64.0f), 40);

	result->CreateVertexBuffer(inRenderer);
	result->CreateIndexBuffer(inRenderer);

	return inAssetManager.Register(result);
}

AssetHandle<StaticMesh> Factory<StaticMesh>::MakeCone(f32 inRadius, f32 inHeight, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager)
{
	StaticMesh* result = new StaticMesh();

	AddConicSurface(*result, { { -0.5f * inHeight, 0.0f, 0.0f }, {} }, inRadius, inHeight, glm::vec2(75.0f / 256.0f), 71.0f / 256.0f, 48, 10);
	AddCircle(*result, { { -0.5f * inHeight, 0.0f, 0.0f }, mage::Rotor({ 0.0f, 0.0f, 1.0f }, glm::radians(180.0f)) }, inRadius, glm::vec2(181.0f / 256.0f), 71.0f / 256.0f, 4);

	result->CreateVertexBuffer(inRenderer);
	result->CreateIndexBuffer(inRenderer);

	return inAssetManager.Register(result);
}

void Factory<StaticMesh>::AddHemisphere(StaticMesh& inOutResult, mage::Transform inTransform, f32 inRadius, glm::vec2 inUvCenter, f32 inUvRadius, u32 inSubdivisions)
{
	u32 indexOffset = inOutResult.mVertices.GetSize();

	glm::vec3 x = inTransform.Rotation.Rotate(glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 y = inTransform.Rotation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 z = inTransform.Rotation.Rotate(glm::vec3(0.0f, 0.0f, 1.0f));

	inOutResult.mVertices.AddConstruct(inTransform.Position + inRadius * x, x, inUvCenter);

	u32 vertexCountPerEdge = 2 * (inSubdivisions + 1);
	f32 angle = glm::atan(2.0f) / vertexCountPerEdge;

	for (u32 i = 0; i < 5; ++i)
	{
		glm::vec3 axisA = mage::Rotor(x, glm::radians(72.0f * i)).Rotate(z);
		glm::vec3 axisB = mage::Rotor(x, glm::radians(72.0f * (i + 1))).Rotate(z);

		glm::vec3 reflector = mage::Rotor(axisA, glm::atan(2.0f)).Rotate(x) + mage::Rotor(axisB, glm::atan(2.0f)).Rotate(x);

		for (u32 a = 1; a <= vertexCountPerEdge; ++a)
		{
			glm::vec3 posA = mage::Rotor(axisA, a * angle).Rotate(x);
			glm::vec3 posB = mage::Rotor(axisB, a * angle).Rotate(x);

			glm::vec3 axisC = glm::normalize(glm::cross(posB, posA));
			f32 angle2 = glm::acos(glm::dot(posB, posA)) / a;

			for (u32 b = 0; b < a; ++b)
			{
				glm::vec3 pos = mage::Rotor(axisC, b * angle2).Rotate(posA);
				glm::vec3 uv = { glm::pow(1.0f - glm::pow(glm::dot(x, pos), 0.75f), 0.2f), glm::dot(y, pos), glm::dot(z, pos), };
				inOutResult.mVertices.AddConstruct(inTransform.Position + inRadius * pos, pos, inUvCenter + inUvRadius * uv.x * glm::vec2(uv.y, -uv.z));

				if (a < vertexCountPerEdge)
				{
					glm::vec3 rpos = 2.0f * glm::dot(pos, reflector) / glm::dot(reflector, reflector) * reflector - pos;
					if (rpos.x < 0.0f)
						rpos = -rpos;

					{
						glm::vec3 ruv = { glm::pow(1.0f - glm::pow(glm::dot(x, rpos), 0.75f), 0.2f), glm::dot(y, rpos), glm::dot(z, rpos), };
						inOutResult.mVertices.AddConstruct(inTransform.Position + inRadius * rpos, rpos, inUvCenter + inUvRadius * ruv.x * glm::vec2(ruv.y, -ruv.z));
					}

					if (rpos.x < 0.0001f)
					{
						rpos = -rpos;
						glm::vec3 ruv = { glm::pow(1.0f - glm::pow(glm::dot(-x, rpos), 0.75f), 0.2f), glm::dot(y, rpos), glm::dot(z, rpos), };
						inOutResult.mVertices.AddConstruct(inTransform.Position + inRadius * rpos, rpos, inUvCenter + inUvRadius * ruv.x * glm::vec2(ruv.y, -ruv.z));
					}
				}
			}
		}
	}

	f32 threshold = 0.995f * glm::dot(inOutResult.mVertices[indexOffset + 1].Normal, inOutResult.mVertices[indexOffset + 1 + vertexCountPerEdge * (2 * vertexCountPerEdge + 1) / 2].Normal);

	for (u32 a = indexOffset + 2; a < inOutResult.mVertices.GetSize(); ++a)
		for (u32 b = indexOffset + 1; b < a; ++b)
		{
			glm::vec3 normA = inOutResult.mVertices[a].Normal;
			glm::vec3 normB = inOutResult.mVertices[b].Normal;

			if (glm::dot(normA, normB) < threshold)
				continue;

			for (u32 c = indexOffset; c < b; ++c)
			{
				glm::vec3 normC = inOutResult.mVertices[c].Normal;

				if (glm::dot(normA, normC) < threshold)
					continue;

				if (glm::dot(normB, normC) < threshold)
					continue;

				if (glm::dot(normA, glm::cross(normC - normA, normB - normA)) > 0.0f)
					inOutResult.mFaces.AddConstruct(a, b, c);
				else
					inOutResult.mFaces.AddConstruct(a, c, b);
			}
		}
}

void Factory<StaticMesh>::AddCircle(StaticMesh& inOutResult, mage::Transform inTransform, f32 inRadius, glm::vec2 inUvCenter, f32 inUvRadius, u32 inSubdivisions)
{
	glm::vec3 x = inTransform.Rotation.Rotate(glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 y = inTransform.Rotation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 z = inTransform.Rotation.Rotate(glm::vec3(0.0f, 0.0f, 1.0f));

	u32 vertexCount = 3 << inSubdivisions;
	u32 indexOffset = inOutResult.mVertices.GetSize();

	for (u32 i = 0; i < vertexCount; ++i)
	{
		glm::vec3 pos = mage::Rotor(x, glm::radians(360.0f * f32(i) / f32(vertexCount))).Rotate(y);
		glm::vec2 uv = inUvCenter + inUvRadius * glm::vec2{ glm::dot(y, pos), -glm::dot(z, pos) };

		inOutResult.mVertices.AddConstruct(inTransform.Position + inRadius * pos, x, uv);
	}

	inOutResult.mFaces.AddConstruct(indexOffset, indexOffset + (1 << inSubdivisions), indexOffset + (2 << inSubdivisions));

	for (u32 i = 1; i <= inSubdivisions; ++i)
	{
		u32 stepSize = 1 << (inSubdivisions - i);
		u32 stepCount = 3 << i;

		for (u32 j = 2; j < stepCount; j += 2)
			inOutResult.mFaces.AddConstruct(indexOffset + stepSize * (j - 2), indexOffset + stepSize * (j - 1), indexOffset + stepSize * j);

		inOutResult.mFaces.AddConstruct(indexOffset, indexOffset + stepSize * (stepCount - 2), indexOffset + stepSize * (stepCount - 1));
	}
}

void Factory<StaticMesh>::AddCylindricSurface(StaticMesh& inOutResult, mage::Transform inTransform, f32 inRadius, f32 inHalfHeight, glm::vec2 inUvMin, glm::vec2 inUvMax, u32 inRadialVertexCount)
{
	glm::vec3 x = inTransform.Rotation.Rotate(glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 y = inTransform.Rotation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f));

	for (u32 i = 0; i <= inRadialVertexCount; ++i)
	{
		glm::vec3 pos = mage::Rotor(x, glm::radians(360.0f * f32(i) / f32(inRadialVertexCount))).Rotate(y);
		f32 uvX = inUvMax.x - (inUvMax.x - inUvMin.x) * f32(i) / f32(inRadialVertexCount);

		if (i > 0)
		{
			u32 indexOffset = inOutResult.mVertices.GetSize();

			inOutResult.mFaces.AddConstruct(indexOffset - 2, indexOffset, indexOffset + 1);
			inOutResult.mFaces.AddConstruct(indexOffset - 2, indexOffset + 1, indexOffset - 1);
		}

		inOutResult.mVertices.AddConstruct(inTransform.Position + inRadius * pos - inHalfHeight * x, pos, glm::vec2(uvX, inUvMax.y));
		inOutResult.mVertices.AddConstruct(inTransform.Position + inRadius * pos + inHalfHeight * x, pos, glm::vec2(uvX, inUvMin.y));
	}
}

void Factory<StaticMesh>::AddFlatSurface(StaticMesh& inOutResult, mage::Transform inTransform, glm::vec2 inHalfExtent, glm::vec2 inUvMin, glm::vec2 inUvMax)
{
	glm::vec3 normal = inTransform.Rotation.Rotate(glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 x = inTransform.Rotation.Rotate(glm::vec3(0.0f, inHalfExtent.x, 0.0f));
	glm::vec3 y = inTransform.Rotation.Rotate(glm::vec3(0.0f, 0.0f, inHalfExtent.y));

	u32 indexOffset = inOutResult.mVertices.GetSize();

	inOutResult.mFaces.AddConstruct(indexOffset, indexOffset + 2, indexOffset + 3);
	inOutResult.mFaces.AddConstruct(indexOffset, indexOffset + 3, indexOffset + 1);

	inOutResult.mVertices.AddConstruct(inTransform.Position - x - y, normal, glm::vec2(inUvMin.x, inUvMax.y));
	inOutResult.mVertices.AddConstruct(inTransform.Position + x - y, normal, glm::vec2(inUvMax.x, inUvMax.y));
	inOutResult.mVertices.AddConstruct(inTransform.Position - x + y, normal, glm::vec2(inUvMin.x, inUvMin.y));
	inOutResult.mVertices.AddConstruct(inTransform.Position + x + y, normal, glm::vec2(inUvMax.x, inUvMin.y));
}

void Factory<StaticMesh>::AddConicSurface(StaticMesh& inOutResult, mage::Transform inTransform, f32 inRadius, f32 inHeight, glm::vec2 inUvCenter, f32 inUvRadius, u32 inRadialVertexCount, u32 inLateralVertexCount)
{
	glm::vec3 x = inTransform.Rotation.Rotate(glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 y = inTransform.Rotation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 z = inTransform.Rotation.Rotate(glm::vec3(0.0f, 0.0f, 1.0f));

	for (u32 i = 0; i <= inRadialVertexCount; ++i)
	{
		glm::vec3 pos = mage::Rotor(x, glm::radians(360.0f * f32(i) / f32(inRadialVertexCount))).Rotate(y);

		f32 offset = 1.0f;

		for (u32 j = 0; j < inLateralVertexCount; ++j)
		{
			if (i > 0 && j > 0)
			{
				u32 a = inOutResult.mVertices.GetSize();
				u32 b = a - 1;
				u32 c = b - inLateralVertexCount;
				u32 d = c - 1;

				inOutResult.mFaces.AddConstruct(d, b, a);
				inOutResult.mFaces.AddConstruct(d, a, c);
			}

			glm::vec2 uv = inUvCenter + inUvRadius * offset * glm::vec2{ glm::dot(y, pos), -glm::dot(z, pos) };
			inOutResult.mVertices.AddConstruct(inTransform.Position + inRadius * offset * pos + inHeight * (1.0f - offset) * x, pos, uv);

			offset *= 0.75f;
		}

		if (i > 0)
		{
			u32 a = inOutResult.mVertices.GetSize();
			u32 b = a - 1;
			u32 c = b - 1 - inLateralVertexCount;

			inOutResult.mFaces.AddConstruct(a, c, b);
		}

		inOutResult.mVertices.AddConstruct(inTransform.Position + inHeight * x, pos, inUvCenter);
	}
}

void Factory<StaticMesh>::AddInvertedCopy(StaticMesh& inOutResult, glm::vec2 inUvOffset)
{
	u32 vertexCount = inOutResult.mVertices.GetSize();
	u32 faceCount = inOutResult.mFaces.GetSize();

	for (u32 i = 0; i < vertexCount; ++i)
	{
		StaticMesh::Vertex const& vertex = inOutResult.mVertices[i];
		inOutResult.mVertices.AddConstruct(-vertex.Position, -vertex.Normal, inUvOffset + glm::vec2(vertex.TextureCoords.x, -vertex.TextureCoords.y));
	}

	for (u32 i = 0; i < faceCount; ++i)
	{
		StaticMesh::Triangle const& face = inOutResult.mFaces[i];
		inOutResult.mFaces.AddConstruct(vertexCount + face.Index[0], vertexCount + face.Index[2], vertexCount + face.Index[1]);
	}
}
