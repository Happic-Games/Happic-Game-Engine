#pragma once

#include "../ECSSystem.h"

namespace Happic { namespace ECS {

	class MouseLookAroundSystem : public System<MouseLookAroundSystem>
	{
	public:
		MouseLookAroundSystem();

		void ProcessEntity(EntityID entityID, Core::IDisplayInput* pInput, float dt);
	};

} }
