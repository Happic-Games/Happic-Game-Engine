#pragma once

#include "../ECSSystem.h"

namespace Happic { namespace ECS {

	class RotationSystem : public System<RotationSystem>
	{
	public:
		RotationSystem();

		void ProcessEntity(EntityID entityID, Core::IDisplayInput* pInput, float dt) override;
	private:
		float m;
	};

} }
