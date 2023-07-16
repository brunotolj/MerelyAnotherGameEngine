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
		std::shared_ptr<Model> model;

		struct Transform
		{
			glm::vec2 Position = { 0.0f,0.0f };
			float Rotation = 0.0f;
			glm::vec2 Scale = { 1.0f,1.0f };

			glm::mat4 Matrix();
		} transform;

		glm::vec3 color = { 1.0f,1.0f,1.0f };
	};
}
