#pragma once

#include "../ECSSystem.h"

namespace Happic { namespace ECS {

	class ViewProjectionSystem : public System<ViewProjectionSystem>
	{
	public:
		ViewProjectionSystem();

		void ProcessEntity(EntityID entityID, Core::IDisplayInput* pInput, float dt) override;
	};

} }
