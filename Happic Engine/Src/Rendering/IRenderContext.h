#pragma once

#include "IDisplay.h"
#include "GraphicsPipeline/GraphicsPipeline.h"
#include "DrawCommand.h"

namespace Happic { namespace Rendering { 

	class IRenderContext;

	class IRenderContext
	{
	public:
		virtual ~IRenderContext() {}

		virtual void Init(IDisplay* pDisplay) = 0;

		virtual void ChangeGraphicsPipeline(const GraphicsPipeline& pipeline) = 0;

		virtual void UpdatePerDrawInstanceBuffer(ShaderType type, const void* pData) = 0;

		virtual void SubmitDrawCommand(const DrawCommand& drawCommand) const = 0;

		virtual void BeginFrame() const = 0;

		virtual void Swap() = 0;
		virtual void DisplayClosed() const {}

		static IRenderContext* CreateRenderContext(IDisplay* pDisplay, RenderAPI renderAPI);
		static void DestroyRenderContext();

		static RenderAPI GetRenderAPI();
		static IRenderContext* GetRenderContext();
	private:
		static RenderAPI s_renderAPI;
		static IRenderContext* s_pRenderContext;
	};

} }
