#include "ECSBaseSystem.h"

namespace Happic { namespace ECS {

	SystemID BaseSystem::s_lastID = NULL_ECS_HANDLE;

	void BaseSystem::AddComponentType(ComponentID componentID)
	{
		m_inputSignature.push_back(componentID);
	}

	void BaseSystem::RemoveComponentType(ComponentID componentID)
	{
		for (uint32 i = 0; i < m_inputSignature.size(); i++)
		{
			if (m_inputSignature[i] == componentID)
			{
				m_inputSignature.erase(m_inputSignature.begin() + i);
				return;
			}
		}
	}

} }