#pragma once

#include "Core/NonCopyable.h"
#include "Rendering/MV_Device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace MV
{
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
		};

		Model(Device& device, const Builder& builder);
		~Model();

		static std::unique_ptr<Model> CreateFromFile(Device& device, const std::string& path);

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
}
