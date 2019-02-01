#include "ITextureSampler.h"
#include "../Platform/Vulkan/TextureSamplerVK.h"
#include "../Platform/OpenGL/TextureSamplerGL.h"
#include "../Platform/D3D11/TextureSamplerD3D11.h"
#include "IRenderContext.h"

namespace Happic { namespace Rendering {

	ITextureSampler * ITextureSampler::CreateTextureSampler(FilterType filterType, AddressMode addressMode, AnisotropyLevel anisotropyLevel)
	{
		ITextureSampler* pSampler = NULL;

		switch (IRenderContext::GetRenderAPI())
		{
		case RENDER_API_VULKAN: pSampler = new TextureSamplerVK(); break;
		case RENDER_API_OPENGL: pSampler = new TextureSamplerGL(); break;
		case RENDER_API_D3D11: pSampler = new TextureSamplerD3D11(); break;
		}

		pSampler->Init(filterType, addressMode, anisotropyLevel);
		return pSampler;
	}

} }
