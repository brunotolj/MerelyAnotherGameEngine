#pragma once

#include "Core/NonCopyable.h"
#include "Core/Rotor.h"
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
		std::shared_ptr<Model> mModel;

		struct Transform
		{
			glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };
			mage::Rotor mRotation = { 1.0f, 0.0f, 0.0f, 0.0f };
			glm::vec3 mScale = { 1.0f, 1.0f, 1.0f };

			glm::mat4 Matrix();
		} mTransform;

		glm::vec3 mColor = { 1.0f, 1.0f, 1.0f };
	};
}
