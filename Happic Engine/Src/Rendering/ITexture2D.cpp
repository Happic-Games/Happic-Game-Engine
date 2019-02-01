#include "ITexture2D.h"
#include "../Vendor/stb_image.h"
#include "../Platform/Vulkan/Texture2DVK.h"
#include "../Platform/OpenGL/Texture2DGL.h"
#include "../Platform/D3D11/Texture2DD3D11.h"
#include "IRenderContext.h"

namespace Happic { namespace Rendering {

	ITexture2D * ITexture2D::CreateTexture(const void * pData, uint32 width, uint32 height)
	{
		ITexture2D * pTexture = NULL;

		switch (IRenderContext::GetRenderAPI())
		{
		case RENDER_API_VULKAN: pTexture = new Texture2DVK(); break;
		case RENDER_API_OPENGL: pTexture = new Texture2DGL(); break;
		case RENDER_API_D3D11: pTexture = new Texture2DD3D11(); break;
		}

		pTexture->Init(pData, width, height);
		return pTexture;
	}

	ITexture2D * ITexture2D::CreateTexture(const String & file)
	{
		int width, height;
		unsigned char* pPixels = stbi_load(file.C_Str(), &width, &height, NULL, 4);
		ITexture2D* pTexture = CreateTexture(pPixels, width, height);
		stbi_image_free(pPixels);
		return pTexture;
	}

} }
