#pragma once

#include "../ECSSystem.h"

namespace Happic { namespace ECS {

	class KeyboardMovementSystem : public System<KeyboardMovementSystem>
	{
	public:
		KeyboardMovementSystem();

		void ProcessEntity(EntityID entityID, Core::IDisplayInput* pInput, float dt) override;
	};

} }
