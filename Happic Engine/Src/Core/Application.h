#pragma once

#include "../ECS/EntityComponentSystem.h"
#include "../Importers\HappmdlImporter.h"

namespace Happic { namespace Core {

	class Application
	{
	public:
		virtual ~Application() {}
		virtual void Init() = 0;
		virtual void FreeResources() {}

		inline void Render() { m_ecs.ProcessSystems(ECS::SYSTEM_PROCESS_ON_RENDER, NULL, 0.0f); }
		inline void Update(IDisplayInput* pInput, float dt) { m_ecs.ProcessSystems(ECS::SYSTEM_PROCESS_ON_UPDATE, pInput, dt); }
	protected:
		ECS::EntityComponentSystem	m_ecs;
		HappmdlImporter				m_modelImporter;
	private:
		friend class CoreEngine;
		
	};

} }
