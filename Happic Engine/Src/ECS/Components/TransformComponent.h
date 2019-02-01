#pragma once

#include "../ECSComponent.h"
#include "../../Math/Transform.h"

namespace Happic { namespace ECS {

	struct TransformComponent : public Component<TransformComponent>
	{
		TransformComponent(const Math::Transform& transform = Math::Transform()) :
			transform(transform)
		{}
		Math::Transform transform;
	};

} }
