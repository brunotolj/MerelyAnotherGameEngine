#pragma once

#include "Assets/AssetManager.h"
#include "Assets/StaticMesh.h"

namespace Vulkan
{
	class Renderer;
}

template<>
class Factory<StaticMesh>
{
public:
	static AssetHandle<StaticMesh> FromFile(mage::StringView inPath, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager);

	static AssetHandle<StaticMesh> MakeBox(glm::vec3 inHalfExtent, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager);
	static AssetHandle<StaticMesh> MakeBall(f32 inRadius, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager);
	static AssetHandle<StaticMesh> MakeCylinder(f32 inRadius, f32 inHalfHeight, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager);
	static AssetHandle<StaticMesh> MakeCapsule(f32 inRadius, f32 inHalfHeight, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager);
	static AssetHandle<StaticMesh> MakeCone(f32 inRadius, f32 inHeight, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager);

private:
	Factory() {}

	static void AddHemisphere(StaticMesh& inOutResult, mage::Transform inTransform, f32 inRadius, glm::vec2 inUvCenter, f32 inUvRadius, u32 inSubdivisions);
	static void AddCircle(StaticMesh& inOutResult, mage::Transform inTransform, f32 inRadius, glm::vec2 inUvCenter, f32 inUvRadius, u32 inSubdivisions);
	static void AddCylindricSurface(StaticMesh& inOutResult, mage::Transform inTransform, f32 inRadius, f32 inHalfHeight, glm::vec2 inUvMin, glm::vec2 inUvMax, u32 inRadialVertexCount);
	static void AddFlatSurface(StaticMesh& inOutResult, mage::Transform inTransform, glm::vec2 inHalfExtent, glm::vec2 inUvMin, glm::vec2 inUvMax);
	static void AddConicSurface(StaticMesh& inOutResult, mage::Transform inTransform, f32 inRadius, f32 inHeight, glm::vec2 inUvCenter, f32 inUvRadius, u32 inRadialVertexCount, u32 inLateralVertexCount);
	static void AddInvertedCopy(StaticMesh& inOutResult, glm::vec2 inUvOffset);
};
