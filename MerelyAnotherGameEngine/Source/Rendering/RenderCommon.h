#pragma once

#include "Core/Transform.h"

#include <glm/vec3.hpp>

struct VkCommandBuffer_T;

class Renderable
{
public:
	virtual mage::Transform GetTransform() const = 0;
	
	virtual glm::vec3 GetColor() const = 0;
	
	virtual void Bind(VkCommandBuffer_T* commandBuffer) const = 0;
	
	virtual void Draw(VkCommandBuffer_T* commandBuffer) const = 0;
};
