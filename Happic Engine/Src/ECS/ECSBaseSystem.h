#pragma once

#include "ECSHandles.h"
#include "ECSBaseComponent.h"
#include "../Core/SubEngines.h"
#include <vector>

namespace Happic { namespace ECS {

	typedef std::vector<ComponentID> ComponentInputSignature;

	class EntityComponentSystem;

	class BaseSystem
	{
	public:
		BaseSystem() {}
		virtual ~BaseSystem() {}
		virtual void ProcessEntity(EntityID entity, Core::IDisplayInput* pInput, float dt) = 0;

		inline const ComponentInputSignature& GetInputSignature() const { return m_inputSignature; }
		inline static SystemID RegisterSystemType() { return ++s_lastID; }
	protected:
		void AddComponentType(ComponentID componentID);
		void RemoveComponentType(ComponentID componentID);
	protected:
		EntityComponentSystem*	m_pECS;
		SubEngines				m_subEngines;
	private:
		ComponentInputSignature m_inputSignature;
		friend class EntityComponentSystem;

		static SystemID s_lastID;
	};

} }
