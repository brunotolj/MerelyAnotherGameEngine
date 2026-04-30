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

	struct ModelInfo
	{
		mage::Array<Vertex> Vertices;
		mage::Array<u32> Indices;
	};

	class Model : public NonMovableClass
	{
	public:
		Model(Renderer const& inRenderer, ModelInfo const& inCreateInfo);

		void Bind(vk::CommandBuffer inCommandBuffer) const;
		void Draw(vk::CommandBuffer inCommandBuffer) const;

		static ModelInfo LoadFromFile(mage::StringView inPath);
		static ModelInfo MakeCube(glm::vec3 inHalfExtent);
		static ModelInfo MakeSphere(f32 inRadius);
		static ModelInfo MakeCylinder(f32 inRadius, f32 inHalfHeight);
		static ModelInfo MakeCapsule(f32 inRadius, f32 inHalfHeight);
		static ModelInfo MakeCone(f32 inRadius, f32 inHeight);

	private:
		void CreateVertexBuffer(Renderer const& inRenderer, mage::Array<Vertex> const& inVertices);
		void CreateIndexBuffer(Renderer const& inRenderer, mage::Array<u32> const& inIndices);

		Buffer mVertexBuffer = nullptr;
		Buffer mIndexBuffer = nullptr;

		u32 mVertexCount = 0;
		u32 mIndexCount = 0;

		static void PopulateIndices(ModelInfo& inModelInfo, mage::Array<std::pair<u32, u32>> const& inSortedEdges);
		static void FixWindingOrders(ModelInfo& inModelInfo);
	};
}
