#include "IBuffer.h"
#include "../Platform/Vulkan/BufferVK.h"
#include "../Platform/D3D11/BufferD3D11.h"
#include "../Platform/OpenGL/BufferGL.h"
#include "IRenderContext.h"

namespace Happic { namespace Rendering {

	IBuffer * IBuffer::CreateBuffer(BufferType type, BufferUsage usage, const void * pData, uint32 size)
	{
		IBuffer* pBuffer = NULL;

		switch (IRenderContext::GetRenderAPI())
		{
		case RENDER_API_VULKAN: pBuffer = new BufferVK(); break;
		case RENDER_API_D3D11: pBuffer = new BufferD3D11(); break;
		case RENDER_API_OPENGL: pBuffer = new BufferGL(); break;
		}

		bool success = pBuffer->Init(type, usage, pData, size);
		if (!success)
			return NULL;

		return pBuffer;
	}

} }
