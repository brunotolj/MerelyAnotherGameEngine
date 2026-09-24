#pragma once

#include "Assets/Asset.h"
#include "Vulkan/Buffer.h"

class StaticMesh : public Asset
{
	friend class Factory<StaticMesh>;

public:
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TextureCoords;

		static mage::Array<vk::VertexInputBindingDescription> GetBindingDescriptions();
		static mage::Array<vk::VertexInputAttributeDescription> GetAttributeDescriptions();

		bool operator==(Vertex const& inOther) const = default;
	};

	struct Triangle
	{
		Triangle() {}
		Triangle(u32 a, u32 b, u32 c) : Index{ a, b, c } {}

		u32 Index[3];
	};

	void Bind(vk::CommandBuffer inCommandBuffer) const;
	void Draw(vk::CommandBuffer inCommandBuffer) const;

private:
	StaticMesh() {}

	void CreateVertexBuffer();
	void CreateIndexBuffer();

	Vulkan::Buffer mVertexBuffer = nullptr;
	Vulkan::Buffer mIndexBuffer = nullptr;

	mage::Array<Vertex> mVertices;
	mage::Array<Triangle> mFaces;
};

template<>
class Factory<StaticMesh>
{
public:
	static AssetHandle<StaticMesh> FromFile(mage::StringView inPath);
	static AssetHandle<StaticMesh> Create(mage::StringView inName, PropertyContainer const& inProperties);

	static AssetHandle<StaticMesh> MakeBox(mage::StringView inName, glm::vec3 inHalfExtent);
	static AssetHandle<StaticMesh> MakeSphere(mage::StringView inName, f32 inRadius);
	static AssetHandle<StaticMesh> MakeCylinder(mage::StringView inName, f32 inRadius, f32 inHalfHeight);
	static AssetHandle<StaticMesh> MakeCapsule(mage::StringView inName, f32 inRadius, f32 inHalfHeight);
	static AssetHandle<StaticMesh> MakeCone(mage::StringView inName, f32 inRadius, f32 inHeight);

private:
	Factory() {}

	static void AddHemisphere(StaticMesh& inOutResult, mage::Transform inTransform, f32 inRadius, glm::vec2 inUvCenter, f32 inUvRadius, u32 inSubdivisions);
	static void AddCircle(StaticMesh& inOutResult, mage::Transform inTransform, f32 inRadius, glm::vec2 inUvCenter, f32 inUvRadius, u32 inSubdivisions);
	static void AddCylindricSurface(StaticMesh& inOutResult, mage::Transform inTransform, f32 inRadius, f32 inHalfHeight, glm::vec2 inUvMin, glm::vec2 inUvMax, u32 inRadialVertexCount);
	static void AddFlatSurface(StaticMesh& inOutResult, mage::Transform inTransform, glm::vec2 inHalfExtent, glm::vec2 inUvMin, glm::vec2 inUvMax);
	static void AddConicSurface(StaticMesh& inOutResult, mage::Transform inTransform, f32 inRadius, f32 inHeight, glm::vec2 inUvCenter, f32 inUvRadius, u32 inRadialVertexCount, u32 inLateralVertexCount);
	static void AddInvertedCopy(StaticMesh& inOutResult, glm::vec2 inUvOffset);
};
