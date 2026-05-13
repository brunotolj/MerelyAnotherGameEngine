#pragma once

#include "Vulkan/Buffer.h"

#include <vulkan/vulkan_raii.hpp>

namespace Vulkan
{
	class Renderer;

	class Model : public NonMovableClass
	{
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

		struct CreateInfo
		{
			mage::Array<Vertex> Vertices;
			mage::Array<Triangle> Faces;
		};

		Model(Renderer const& inRenderer, CreateInfo const& inCreateInfo);

		void Bind(vk::CommandBuffer inCommandBuffer) const;
		void Draw(vk::CommandBuffer inCommandBuffer) const;

		static CreateInfo LoadFromFile(mage::StringView inPath);
		static CreateInfo MakeBox(glm::vec3 inHalfExtent);
		static CreateInfo MakeBall(f32 inRadius);
		static CreateInfo MakeCylinder(f32 inRadius, f32 inHalfHeight);
		static CreateInfo MakeCapsule(f32 inRadius, f32 inHalfHeight);
		static CreateInfo MakeCone(f32 inRadius, f32 inHeight);

		static void AddHemisphere(CreateInfo& inOutResult, mage::Transform inTransform, f32 inRadius, glm::vec2 inUvCenter, f32 inUvRadius, u32 inSubdivisions);
		static void AddCircle(CreateInfo& inOutResult, mage::Transform inTransform, f32 inRadius, glm::vec2 inUvCenter, f32 inUvRadius, u32 inSubdivisions);
		static void AddCylindricSurface(CreateInfo& inOutResult, mage::Transform inTransform, f32 inRadius, f32 inHalfHeight, glm::vec2 inUvMin, glm::vec2 inUvMax, u32 inRadialVertexCount);
		static void AddFlatSurface(CreateInfo& inOutResult, mage::Transform inTransform, glm::vec2 inHalfExtent, glm::vec2 inUvMin, glm::vec2 inUvMax);
		static void AddConicSurface(CreateInfo& inOutResult, mage::Transform inTransform, f32 inRadius, f32 inHeight, glm::vec2 inUvCenter, f32 inUvRadius, u32 inRadialVertexCount, u32 inLateralVertexCount);
		static void AddInvertedCopy(CreateInfo& inOutResult, glm::vec2 inUvOffset);

	private:
		void CreateVertexBuffer(Renderer const& inRenderer, mage::Array<Vertex> const& inVertices);
		void CreateIndexBuffer(Renderer const& inRenderer, mage::Array<Triangle> const& inFaces);

		Buffer mVertexBuffer = nullptr;
		Buffer mIndexBuffer = nullptr;

		u32 mVertexCount = 0;
		u32 mIndexCount = 0;
	};
}
