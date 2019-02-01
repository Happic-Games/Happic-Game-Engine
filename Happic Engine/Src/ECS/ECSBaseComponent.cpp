#include "ECSBaseComponent.h"

namespace Happic { namespace ECS {

	std::vector<uint32> BaseComponent::s_registeredComponentTypes;

	BaseComponent::BaseComponent() :
		entity(NULL_ECS_HANDLE)
	{}

	ComponentID BaseComponent::RegisterComponentType(uint32 size)
	{
		ComponentID id = s_registeredComponentTypes.size();
		s_registeredComponentTypes.push_back(size);
		return id;
	}

	uint32 BaseComponent::GetComponentSize(ComponentID type)
	{
		return s_registeredComponentTypes[type];
	}



} }