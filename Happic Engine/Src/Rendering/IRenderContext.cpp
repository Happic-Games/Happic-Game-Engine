#include "IRenderContext.h"
#include "../Platform/Vulkan/RenderContextVK.h"
#include "../Platform/D3D11/RenderContextD3D11.h"
#include "../Platform/OpenGL/RenderContextGL.h"

namespace Happic { namespace Rendering { 

	RenderAPI IRenderContext::s_renderAPI = RENDER_API_NONE;
	IRenderContext* IRenderContext::s_pRenderContext = NULL;

	IRenderContext* IRenderContext::CreateRenderContext(IDisplay* pDisplay, RenderAPI renderAPI)
	{
		s_renderAPI = renderAPI;

		switch (renderAPI)
		{
		case RENDER_API_VULKAN: s_pRenderContext = new RenderContextVK(); break;
		case RENDER_API_D3D11: s_pRenderContext = new RenderContextD3D11(); break;
		case RENDER_API_OPENGL: s_pRenderContext = new RenderContextGL(); break;
		}

		s_pRenderContext->Init(pDisplay);
		return s_pRenderContext;
	}

	void IRenderContext::DestroyRenderContext()
	{
		s_renderAPI = RENDER_API_NONE;
		delete s_pRenderContext;
	}

	RenderAPI IRenderContext::GetRenderAPI()
	{
		return s_renderAPI;
	}

	IRenderContext * IRenderContext::GetRenderContext()
	{
		return s_pRenderContext;
	}

} }
