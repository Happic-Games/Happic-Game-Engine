#pragma once

#include "../ECSComponent.h"
#include "../../Math/Math.h"

namespace Happic { namespace ECS {

	struct RotationComponent : public Component<RotationComponent>
	{
		RotationComponent(const Math::Vector3f& axis, float rotationPerSecond, float circleRadius, float revolutionPerSecond) :
			axis(axis),
			rotationPerSecond(rotationPerSecond),
			circleRadius(circleRadius),
			revolutionPerSecond(revolutionPerSecond)
		{}

		Math::Vector3f		axis;
		float				rotationPerSecond;
		float				circleRadius;
		float				revolutionPerSecond;
	};

} }