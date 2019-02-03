#pragma once

#include "../DataStructures/Types.h"
#include "ITexture2D.h"

#define FRAMEBUFFER_MAX_RENDER_TEXTURES 8

namespace Happic { namespace Rendering {

	class IFramebuffer
	{
	public:
		virtual void AddRenderTexture(TextureFormat format) = 0;
		virtual void AddDepthTexture(TextureFormat format) = 0;

		virtual void Create() = 0;

		virtual void PrepareForReading(bool readDepthBuffer) const = 0;
		virtual void PrepareForRendering() const = 0;

		virtual uint32 GetWidth() const = 0;
		virtual uint32 GetHeight() const = 0;

		virtual bool HasDepth() const = 0;
	};

} }
