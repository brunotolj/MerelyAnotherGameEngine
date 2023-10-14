#pragma once

#include "Core/NonCopyable.h"
#include "MV/MV_Model.h"
#include "MV/MV_TransformComponent.h"

#include <memory>

namespace MV
{
	class Object : public NonCopyableClass
	{
	public:	
		std::shared_ptr<Model> mModel;

		TransformComponent mTransformComponent;

		glm::vec3 mColor = { 1.0f, 1.0f, 1.0f };
	};
}
