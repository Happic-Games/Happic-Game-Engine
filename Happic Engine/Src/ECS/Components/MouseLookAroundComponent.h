#pragma once

#include "../ECSComponent.h"
#include "../../Core/InputDefs.h"

namespace Happic { namespace ECS {

	struct MouseLookAroundComponent : public Component<MouseLookAroundComponent>
	{
		MouseLookAroundComponent(Key toggleKey, float sensitivityX, float sensitivityY) :
			toggleKey(toggleKey),
			sensitivityX(sensitivityX),
			sensitivityY(sensitivityY)
		{}

		Key		toggleKey;
		float	sensitivityX;
		float	sensitivityY;
	};

} }
