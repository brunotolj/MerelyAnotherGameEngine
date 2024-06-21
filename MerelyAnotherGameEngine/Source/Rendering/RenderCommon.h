#pragma once

#include "Core/Transform.h"

#include <glm/vec3.hpp>

struct VkCommandBuffer_T;

class IRenderable
{
public:
	virtual mage::Transform GetTransform() const = 0;
	
	virtual glm::vec3 GetColor() const = 0;
	
	virtual void Bind(VkCommandBuffer_T* commandBuffer) const = 0;
	
	virtual void Draw(VkCommandBuffer_T* commandBuffer) const = 0;
};

class ICamera
{
public:
	virtual glm::mat4 GetViewTransform() const = 0;

	virtual const glm::mat4& GetProjectionTransform() const = 0;
};
