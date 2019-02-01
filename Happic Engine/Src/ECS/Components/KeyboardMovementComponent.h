#pragma once

#include "../ECSComponent.h"
#include "../../Core/InputDefs.h"

namespace Happic { namespace ECS {

	struct KeyboardMovementComponent : public Component<KeyboardMovementComponent>
	{
		KeyboardMovementComponent(Key moveForward, Key moveBack, Key moveLeft, Key moveRight, Key sprintKey, float speed, float sprintMultiplier) :
			moveForward(moveForward),
			moveBack(moveBack),
			moveLeft(moveLeft),
			moveRight(moveRight),
			sprintKey(sprintKey),
			speed(speed),
			sprintMultiplier(sprintMultiplier)
		{}

		Key		moveForward;
		Key		moveBack;
		Key		moveLeft;
		Key		moveRight;
		Key		sprintKey;
		float	speed;
		float	sprintMultiplier;
	};

} }
