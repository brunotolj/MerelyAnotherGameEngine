#pragma once

#include "Rendering/Device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Buffer;

class Model : public NonCopyableClass
{
public:
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 UV;

		static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

		bool operator==(const Vertex& other) const = default;
	};

	struct Builder
	{
		std::vector<Vertex> Vertices{};
		std::vector<u32> Indices{};

		void LoadModel(const std::string& path);

		void MakeCube(f32 halfExtentX, f32 halfExtentY, f32 halfExtentZ);

		void MakeSphere(f32 radius);

		void MakeCylinder(f32 radius, f32 halfHeight);

		void MakeCapsule(f32 radius, f32 halfHeight);

		void FixWindingOrders();
	};

	Model(Device& device, const Builder& builder);
	~Model();

	static std::shared_ptr<Model> CreateFromFile(Device& device, const std::string& path);
	
	static std::shared_ptr<Model> CreateCube(Device& device, f32 halfExtentX, f32 halfExtentY, f32 halfExtentZ);
	
	static std::shared_ptr<Model> CreateSphere(Device& device, f32 radius);
	
	static std::shared_ptr<Model> CreateCylinder(Device& device, f32 radius, f32 halfHeight);
	
	static std::shared_ptr<Model> CreateCapsule(Device& device, f32 radius, f32 halfHeight);

	void Bind(VkCommandBuffer commandBuffer) const;

	void Draw(VkCommandBuffer commandBuffer) const;

private:
	Device& mDevice;

	std::unique_ptr<Buffer> mVertexBuffer;
	u32 mVertexCount;

	std::unique_ptr<Buffer> mIndexBuffer;
	u32 mIndexCount;

	void CreateVertexBuffer(const std::vector<Vertex>& vertices);
	void CreateIndexBuffer(const std::vector<u32>& indices);
};
