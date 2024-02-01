#pragma once

#include "Core/NonCopyable.h"
#include "Rendering/Device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

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
		std::vector<uint32_t> Indices{};

		void LoadModel(const std::string& path);

		void MakeCube(float halfExtentX, float halfExtentY, float halfExtentZ);

		void MakeSphere(float radius);

		void MakeCylinder(float radius, float halfHeight);

		void MakeCapsule(float radius, float halfHeight);
	};

	Model(Device& device, const Builder& builder);
	~Model();

	static std::unique_ptr<Model> CreateFromFile(Device& device, const std::string& path);

	static std::unique_ptr<Model> CreateCube(Device& device, float halfExtentX, float halfExtentY, float halfExtentZ);

	static std::unique_ptr<Model> CreateSphere(Device& device, float radius);

	static std::unique_ptr<Model> CreateCylinder(Device& device, float radius, float halfHeight);

	static std::unique_ptr<Model> CreateCapsule(Device& device, float radius, float halfHeight);

	void Bind(VkCommandBuffer commandBuffer);

	void Draw(VkCommandBuffer commandBuffer);

private:
	Device& mDevice;

	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	uint32_t mVertexCount;

	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;
	uint32_t mIndexCount;

	void CreateVertexBuffers(const std::vector<Vertex>& vertices);
	void CreateIndexBuffers(const std::vector<uint32_t>& indices);
};
