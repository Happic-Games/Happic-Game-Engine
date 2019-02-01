#include "RenderingEngine.h"

namespace Happic { namespace Rendering {

	RenderingEngine::RenderingEngine()
	{
		m_viewProjection.InitIdentity();
	}

	void RenderingEngine::Submit(const Mesh& mesh, const Math::Matrix4f& modelMatrix)
	{
		Math::Matrix4f mvp = m_viewProjection * modelMatrix;
		mvp = mvp.GetTransposed();

		IRenderContext::GetRenderContext()->UpdatePerDrawInstanceBuffer(SHADER_TYPE_VERTEX, &mvp);
		IRenderContext::GetRenderContext()->SubmitDrawCommand(mesh);
	}

	void RenderingEngine::SetViewProjection(const Math::Matrix4f & viewProjection)
	{
		m_viewProjection = viewProjection;
	}

} }
