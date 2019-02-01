#pragma once

#include "ECSHandles.h"
#include <vector>

namespace Happic { namespace ECS {

	struct BaseComponent
	{
	public:
		BaseComponent();
		static ComponentID RegisterComponentType(uint32 size);
		static uint32 GetComponentSize(ComponentID type);

		EntityID entity;
	private:
		static std::vector<uint32> s_registeredComponentTypes;
	};

} }
