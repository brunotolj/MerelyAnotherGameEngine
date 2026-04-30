#pragma once

#include "Vulkan/Buffer.h"

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	class Renderer;

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 UV;

		static mage::Array<vk::VertexInputBindingDescription> GetBindingDescriptions();
		static mage::Array<vk::VertexInputAttributeDescription> GetAttributeDescriptions();

		bool operator==(const Vertex& other) const = default;
	};

	struct ModelCreateInfo
	{
		mage::Array<Vertex> Vertices;
		mage::Array<u32> Indices;
	};

	class Model : public NonMovableClass
	{
	public:
		Model(Renderer const& inRenderer, ModelCreateInfo const& inCreateInfo);

		void Bind(vk::CommandBuffer inCommandBuffer) const;
		void Draw(vk::CommandBuffer inCommandBuffer) const;

		static ModelCreateInfo LoadFromFile(mage::StringView inPath);
		static ModelCreateInfo MakeCube(glm::vec3 inHalfExtent);
		static ModelCreateInfo MakeSphere(f32 inRadius);
		static ModelCreateInfo MakeCylinder(f32 inRadius, f32 inHalfHeight);
		static ModelCreateInfo MakeCapsule(f32 inRadius, f32 inHalfHeight);
		static ModelCreateInfo MakeCone(f32 inRadius, f32 inHeight);

	private:
		void CreateVertexBuffer(Renderer const& inRenderer, mage::Array<Vertex> const& inVertices);
		void CreateIndexBuffer(Renderer const& inRenderer, mage::Array<u32> const& inIndices);

		Buffer mVertexBuffer = nullptr;
		Buffer mIndexBuffer = nullptr;

		u32 mVertexCount = 0;
		u32 mIndexCount = 0;

		static void PopulateIndices(ModelCreateInfo& inModelInfo, mage::Array<std::pair<u32, u32>> const& inSortedEdges);
		static void FixWindingOrders(ModelCreateInfo& inModelInfo);
	};
}
