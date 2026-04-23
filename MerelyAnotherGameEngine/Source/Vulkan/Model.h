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
		static ModelInfo MakeCube(f32 halfExtentX, f32 halfExtentY, f32 halfExtentZ);
		static ModelInfo MakeSphere(f32 radius);
		static ModelInfo MakeCylinder(f32 radius, f32 halfHeight);
		static ModelInfo MakeCapsule(f32 radius, f32 halfHeight);

		static void FixWindingOrders(ModelInfo& inModelInfo);

	private:
		void CreateVertexBuffer(Renderer const& inRenderer, mage::Array<Vertex> const& inVertices);
		void CreateIndexBuffer(Renderer const& inRenderer, mage::Array<u32> const& inIndices);

		Buffer mVertexBuffer = nullptr;
		Buffer mIndexBuffer = nullptr;

		u32 mVertexCount = 0;
		u32 mIndexCount = 0;
	};
}
