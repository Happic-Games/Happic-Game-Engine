#include "EntityComponentSystem.h"

namespace Happic { namespace ECS {

	EntityComponentSystem::EntityComponentSystem() :
		m_ecsAllocator(ECS_MEMORY_CAPACITY),
		m_componentAllocator(m_ecsAllocator.Allocate(ECS_COMPONENT_MEMORY_CAPACITY), ECS_COMPONENT_MEMORY_CAPACITY),
		m_systemAllocator(m_ecsAllocator.Allocate(ECS_SYSTEM_MEMORY_CAPACITY), ECS_SYSTEM_MEMORY_CAPACITY),
		m_lastEntityID(NULL_ECS_HANDLE)
	{}

	EntityComponentSystem::~EntityComponentSystem()
	{
		
	}

	EntityID EntityComponentSystem::CreateEntity()
	{
		EntityID id = NULL_ECS_HANDLE;
		if (!m_freeEntityIDs.empty())
		{
			id = m_freeEntityIDs[m_freeEntityIDs.size() - 1];
			m_freeEntityIDs.pop_back();
		}

		id = ++m_lastEntityID;
		std::vector<ComponentContainer> components;
		m_components[id - 1] = components;
		return id;
	}

	DeleteEntityStatus EntityComponentSystem::DeleteEntity(EntityID entityID)
	{
		if (entityID == NULL_ECS_HANDLE)
			return DELETE_ENTITY_NOT_FOUND;

		const auto&& it = m_components.find(entityID - 1);
		if (it == m_components.end())
			return DELETE_ENTITY_NOT_FOUND;

		const std::vector<ComponentContainer>& components = it->second;
		for (uint32 i = 0; i < components.size(); i++)
		{
			PoolAllocator& componentTypeAllocator = m_componentTypeAllocators[components[i].componentID];
			BaseComponent* pComponent = components[i].pComponent;
			componentTypeAllocator.Free(pComponent);
		}

		m_components.erase(it);
	}

	void EntityComponentSystem::ProcessSystems(SystemProcessType processType, Core::IDisplayInput* pInput, float dt)
	{
		for (uint32 i = 0; i < m_systems[processType].size(); i++)
			ProcessSystem(m_systems[processType][i], pInput, dt);
	}

	void EntityComponentSystem::ProcessSystem(const SystemContainer & system, Core::IDisplayInput* pInput, float dt)
	{
		const ComponentInputSignature& inputSignature = system.pSystem->GetInputSignature();
		for (const auto& entity : m_components)
		{
			const std::vector<ComponentContainer>& components = entity.second;
			if (components.size() < inputSignature.size())
				continue;

			bool compatible = true;
			for (ComponentID requiredID : inputSignature)
			{
				bool foundID = false;

				for (const ComponentContainer& component : components)
				{
					if (requiredID == component.componentID)
					{
						foundID = true;
						break;
					}
				}

				if (!foundID)
				{
					compatible = false;
					break;
				}
			}

			if (compatible)
				system.pSystem->ProcessEntity(entity.first + 1, pInput, dt);
		}
	}
} }
