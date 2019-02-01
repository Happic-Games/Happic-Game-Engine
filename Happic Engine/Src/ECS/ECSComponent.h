#pragma once

#include "ECSBaseComponent.h"

namespace Happic { namespace ECS { 


	template<typename T, uint32 MaxInstances = 512>
	struct Component : public BaseComponent
	{
		virtual ~Component() {}

		static const ComponentID	ID;
		static const uint32			MAX_INSTANCES;
	};

	template<typename T, uint32 MaxInstances>
	const ComponentID Component<T, MaxInstances>::ID = BaseComponent::RegisterComponentType(sizeof(T));

	template<typename T, uint32 MaxInstances>
	const ComponentID Component<T, MaxInstances>::MAX_INSTANCES = MaxInstances;

} }