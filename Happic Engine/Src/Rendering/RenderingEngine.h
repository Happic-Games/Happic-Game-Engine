#pragma once

#include "IRenderContext.h"
#include "Mesh.h"
#include "../Math/Math.h"

namespace Happic { namespace Rendering {

	class RenderingEngine
	{
	public:
		RenderingEngine();

		void Submit(const Mesh& mesh, const Math::Matrix4f& modelMatrix);

		void SetViewProjection(const Math::Matrix4f& viewProjection);
	private:
		Math::Matrix4f			m_viewProjection;
		GraphicsPipeline		m_gbufferPipelineSettings;
	};

} }
