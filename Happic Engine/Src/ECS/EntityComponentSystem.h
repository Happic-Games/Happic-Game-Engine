#pragma once

#include "ECSHandles.h"
#include "../Memory/LinearAllocator.h"
#include "../Memory/PoolAllocator.h"
#include "ECSBaseComponent.h"
#include "ECSBaseSystem.h"
#include <unordered_map>

#define ECS_MAX_COMPONENT_TYPES						128												
#define ECS_MEMORY_CAPACITY							MEMORY_HALF_GB									

#define ECS_COMPONENT_MEMORY_CAPACITY_PORTION		0.75f											
#define ECS_SYSTEM_MEMORY_CAPACITY_PORTION			0.25f

#define ECS_COMPONENT_MEMORY_CAPACITY				ECS_MEMORY_CAPACITY * ECS_COMPONENT_MEMORY_CAPACITY_PORTION
#define ECS_SYSTEM_MEMORY_CAPACITY					ECS_MEMORY_CAPACITY * ECS_SYSTEM_MEMORY_CAPACITY_PORTION

namespace Happic { namespace Core { class CoreEngine; } }

namespace Happic { namespace ECS {

	enum DeleteEntityStatus
	{
		DELETE_ENTITY_SUCCESS,
		DELETE_ENTITY_NOT_FOUND
	};

	enum CreateComponentStatus
	{
		CREATE_COMPONENT_SUCCESS,
		CREATE_COMPONENT_ENTITY_NOT_FOUND,
		CREATE_COMPONENT_COULD_NOT_ALLOCATE_MEMORY,
		CREATE_COMPONENT_COMPONENT_ALREADY_EXISTS
	};

	struct ComponentContainer
	{
		ComponentID		componentID;
		BaseComponent*	pComponent;
	};

	struct SystemContainer
	{
		SystemID		systemID;
		BaseSystem*		pSystem;
	};

	enum SystemProcessType
	{
		SYSTEM_PROCESS_ON_UPDATE,
		SYSTEM_PROCESS_ON_RENDER,

		NUM_SYSTEM_PROCESS_TYPES
	};

	class EntityComponentSystem
	{
	public:
		EntityComponentSystem();
		~EntityComponentSystem();

		EntityID CreateEntity();
		DeleteEntityStatus DeleteEntity(EntityID entityID);

		template<class Comp, class... Args>
		inline CreateComponentStatus CreateComponent(EntityID entityID, Args&&... componentCreateArgs)
		{
			if (entityID == NULL_ECS_HANDLE)
				return CREATE_COMPONENT_ENTITY_NOT_FOUND;

			const ComponentID id = Comp::ID;
			const uint32 maxInstances = Comp::MAX_INSTANCES;
			const uint32 componentSize = BaseComponent::GetComponentSize(id);

			const auto&& it = m_components.find(entityID - 1);
			if (it == m_components.end())
				return CREATE_COMPONENT_ENTITY_NOT_FOUND;

			if (m_componentTypeAllocators.find(id) == m_componentTypeAllocators.end())
			{
				void* pTypeAllocatorMemory = m_componentAllocator.Allocate(maxInstances * componentSize);

				if (!pTypeAllocatorMemory)
					return CREATE_COMPONENT_COULD_NOT_ALLOCATE_MEMORY;

				PoolAllocator typeAllocator(pTypeAllocatorMemory, maxInstances, componentSize);
				m_componentTypeAllocators[id] = typeAllocator;
			}

			ComponentContainer container;
			container.componentID = id;
			void* pComponentMemory = m_componentTypeAllocators[id].Allocate();
			container.pComponent = new(pComponentMemory) Comp(std::forward<Args>(componentCreateArgs)...);
			container.pComponent->entity = entityID;

			it->second.push_back(container);

			return CREATE_COMPONENT_SUCCESS;
		}

		template<class Comp>
		inline Comp* GetComponent(EntityID entityID) const
		{
			if (entityID == NULL_ECS_HANDLE)
				return NULL;

			const auto&& it = m_components.find(entityID - 1);
			if (it == m_components.end())
				return NULL;

			for (uint32 i = 0; i < it->second.size(); i++)
				if (it->second[i].componentID == Comp::ID)
					return (Comp*)it->second[i].pComponent;

			return NULL;
		}

		template<class Comp>
		inline void DeleteComponent(EntityID entityID)
		{
			if (entityID == NULL_ECS_HANDLE)
				return;

			ComponentID componentID = Comp::ID;
			const auto&& it = m_componentTypeAllocators.find(componentID);

			if (it == m_componentTypeAllocators.end())
				return;

			std::vector<ComponentContainer>& components = m_components[entityID - 1];
			for (uint32 i = 0; i < components.size(); i++)
			{
				if (components[i].componentID == componentID)
				{
					BaseComponent* pComponent = components[i].pComponent;
					it->second.Free(pComponent);
					components.erase(components.begin() + i);
				}
			}
		}

		template<class Sys, class... Args>
		inline void CreateSystem(SystemProcessType processType, Args&&... systemCreateArgs)
		{
			uint32 id = Sys::ID;
			uint32 size = Sys::SIZE;

			SystemContainer container;
			void* pSystemMemory = m_systemAllocator.Allocate(size);
			container.pSystem = new(pSystemMemory) Sys(std::forward<Args>(systemCreateArgs)...);
			container.pSystem->m_pECS = this;
			container.pSystem->m_subEngines = m_subEngines;
			container.systemID = id;
			m_systems[processType].push_back(container);
		}


		template<class Sys>
		inline Sys* GetSystem(SystemProcessType processType)
		{
			for (uint32 i = 0; i < m_systems[processType].size(); i++)
				if (m_systems[processType][i].s == Sys::ID)
					return (Sys*)m_systems[processType][i].pSystem;
			return NULL;
		}

		void ProcessSystems(SystemProcessType processType, Core::IDisplayInput* pInput, float dt);
	private:
		void ProcessSystem(const SystemContainer& system, Core::IDisplayInput* pInput, float dt);
	private:
		LinearAllocator														m_ecsAllocator;
		LinearAllocator														m_componentAllocator;
		LinearAllocator														m_systemAllocator;
		std::unordered_map<ComponentID, PoolAllocator>						m_componentTypeAllocators;
		std::unordered_map< EntityID, std::vector<ComponentContainer> >		m_components;
		std::vector<SystemContainer>										m_systems[NUM_SYSTEM_PROCESS_TYPES];
		EntityID															m_lastEntityID;
		std::vector<EntityID>												m_freeEntityIDs;

		friend Core::CoreEngine;
		SubEngines															m_subEngines;
	};

} }
