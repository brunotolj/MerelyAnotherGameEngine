#pragma once

#include "Assets/StaticMesh.h"

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
