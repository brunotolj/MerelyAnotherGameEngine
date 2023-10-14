#pragma once

#include "Core/NonCopyable.h"
#include "Core/Transform.h"

namespace MV
{
	class TransformComponent : public NonCopyableClass
	{
	public:	
		mage::Transform mTransform;
	};
}
