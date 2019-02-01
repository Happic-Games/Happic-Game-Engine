#include "CoreEngine.h"

namespace Happic { namespace Core { 

	CoreEngine::CoreEngine(Application * pApplication, Rendering::RenderAPI renderAPI, const String & title, uint32 width, uint32 height)
	{
		m_pApplication = pApplication;
		m_pDisplay = Rendering::IDisplay::CreateDisplay(title, width, height);
		Rendering::DisplayError err = m_pDisplay->Init(renderAPI);

		if (!err.success)
		{
			std::cout << err.errMsg << std::endl;
			return;
		}

		Rendering::IRenderContext::CreateRenderContext(m_pDisplay, renderAPI, Rendering::GraphicsPipeline::CreateDefaultGraphicsPipeline(width, height));
		m_pRenderingEngine = new Rendering::RenderingEngine();
	}

	CoreEngine::~CoreEngine()
	{
		delete m_pRenderingEngine;
		m_pApplication->FreeResources();
		Rendering::IRenderContext::DestroyRenderContext();
		delete m_pDisplay;
		//delete m_pApplication;
	}

	void CoreEngine::Start()
	{
		if (m_running)
			return;
		m_running = true;

		m_pApplication->m_ecs.m_subEngines.pRenderingEngine = m_pRenderingEngine;
		m_pApplication->Init();

		float dt = 1.0f / 60.0f;
		while (m_running)
		{
			Rendering::WindowEventSystem::ResetWindowEvents();
			m_pDisplay->Update();

			if (Rendering::WindowEventSystem::CheckForWindowEvent(Rendering::WINDOW_EVENT_CLOSED))
			{
				Stop();
				break;
			}

			Update(dt);
			Render();
		}
	}

	void CoreEngine::Stop()
	{
		if (!m_running)
			return;
		m_running = false;
		Rendering::IRenderContext::GetRenderContext()->DisplayClosed();
	}

	void CoreEngine::Update(float dt)
	{
		m_pApplication->Update(m_pDisplay->GetInput(), dt);
		m_pDisplay->GetInput()->Update();
	}

	void CoreEngine::Render()
	{
		Rendering::IRenderContext::GetRenderContext()->BeginFrame();
		m_pApplication->Render();
		Rendering::IRenderContext::GetRenderContext()->Swap();
	}


} }