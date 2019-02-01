#pragma once

#include "Application.h"
#include "../Rendering/IDisplay.h"
#include "../Rendering/IRenderContext.h"
#include "../Rendering/RenderingEngine.h"

namespace Happic { namespace Core {

	class CoreEngine
	{
	public:
		CoreEngine(Application* pApplication, Rendering::RenderAPI renderAPI, const String& title, uint32 width, uint32 height);
		~CoreEngine();

		void Start();
		void Stop();
	private:
		void Update(float dt);
		void Render();
	private:
		Application* m_pApplication;
		Rendering::RenderingEngine* m_pRenderingEngine;
		Rendering::IDisplay* m_pDisplay;
		bool m_running;
	};

} }
