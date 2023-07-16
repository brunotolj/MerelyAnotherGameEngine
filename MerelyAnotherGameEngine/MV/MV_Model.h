#pragma once

#include "NonCopyable.h"
#include "MV/MV_Device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace MV
{
	class Model : public NonCopyableClass
	{
	public:
		struct Vertex
		{
			glm::vec2 pubPosition;
			glm::vec3 pubColor;

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
		};

		Model(Device& device, const std::vector<Vertex>& vertices);
		~Model();

		void Bind(VkCommandBuffer commandBuffer);

		void Draw(VkCommandBuffer commandBuffer);

	private:
		Device& privDevice;

		VkBuffer privVertexBuffer;
		VkDeviceMemory privVertexBufferMemory;

		uint32_t privVertexCount;

		void CreateVertexBuffers(const std::vector<Vertex>& vertices); 
	};
}
