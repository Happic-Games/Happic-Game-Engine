#pragma once

#include "ECSBaseSystem.h"

namespace Happic { namespace ECS {

	
	template<class S>
	class System : public BaseSystem
	{
	public:
		virtual ~System() {}

		static const SystemID	ID;
		static const uint32		SIZE;
	};

	template<class S>
	const SystemID System<S>::ID = BaseSystem::RegisterSystemType();

	template<class S>
	const SystemID System<S>::SIZE = sizeof(S);
} }
