#pragma once

#include "../ECSComponent.h"
#include "../../Math/Math.h"

namespace Happic { namespace ECS {

	struct CameraComponent : public Component<CameraComponent>
	{
		CameraComponent(float fov, float znear = 0.01f, float zfar = 1000.0f) :
			fov(fov),
			znear(znear),
			zfar(zfar)
		{}

		float				fov;
		float				znear;
		float				zfar;
	};

} }
