#pragma once

#include "IDisplay.h"
#include "GraphicsPipeline/GraphicsPipeline.h"
#include "DrawCommand.h"

namespace Happic { namespace Rendering { 

	struct RenderContextInitInfo
	{
		RenderContextInitInfo(IDisplay* pDisplay, const GraphicsPipeline& pipeline) :
			pDisplay(pDisplay),
			pipeline(pipeline)
		{}

		IDisplay* pDisplay;
		GraphicsPipeline pipeline;
	};

	class IRenderContext;

	class IRenderContext
	{
	public:
		virtual ~IRenderContext() {}

		virtual void Init(const RenderContextInitInfo& initInfo) = 0;

		virtual void UpdatePerDrawInstanceBuffer(ShaderType type, const void* pData) = 0;

		virtual void SubmitDrawCommand(const DrawCommand& drawCommand) const = 0;

		virtual void BeginFrame() const = 0;

		virtual void Swap() = 0;
		virtual void DisplayClosed() const {}

		static IRenderContext* CreateRenderContext(IDisplay* pDisplay, RenderAPI renderAPI, const GraphicsPipeline& pipeline);
		static void DestroyRenderContext();

		static RenderAPI GetRenderAPI();
		static IRenderContext* GetRenderContext();
	private:
		static RenderAPI s_renderAPI;
		static IRenderContext* s_pRenderContext;
	};

} }
