#pragma once

#include "Assets/Asset.h"
#include "Vulkan/Buffer.h"

namespace Vulkan
{
	class Renderer;
}

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

	void CreateVertexBuffer(Vulkan::Renderer const& inRenderer);
	void CreateIndexBuffer(Vulkan::Renderer const& inRenderer);

	Vulkan::Buffer mVertexBuffer = nullptr;
	Vulkan::Buffer mIndexBuffer = nullptr;

	mage::Array<Vertex> mVertices;
	mage::Array<Triangle> mFaces;
};
