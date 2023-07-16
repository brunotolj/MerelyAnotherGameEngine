#pragma once

#include "NonCopyable.h"
#include "MV/MV_Model.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

#include <memory>

namespace MV
{
	class Object : public NonCopyableClass
	{
	public:	
		std::shared_ptr<Model> pubModel;

		struct Transform
		{
			glm::vec2 pubPosition = { 0.0f,0.0f };
			float pubRotation = 0.0f;
			glm::vec2 pubScale = { 1.0f,1.0f };

			glm::mat4 Matrix();
		} pubTransform;

		glm::vec3 pubColor = { 1.0f,1.0f,1.0f };
	};
}
