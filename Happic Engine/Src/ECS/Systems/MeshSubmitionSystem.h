#pragma once

#include "../ECSSystem.h"

namespace Happic { namespace ECS {

	class MeshSubmitionSystem : public System<MeshSubmitionSystem>
	{
	public:
		MeshSubmitionSystem();

		void ProcessEntity(EntityID entity, Core::IDisplayInput* pInput, float dt) override;
	};

} }
