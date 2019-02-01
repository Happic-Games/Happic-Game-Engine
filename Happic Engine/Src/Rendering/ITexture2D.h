#pragma once

#include "ITexture.h"
#include "../DataStructures/Types.h"
#include "../DataStructures/String.h"

namespace Happic { namespace Rendering {

	class ITexture2D : public ITexture
	{
	public:
		virtual ~ITexture2D() {}
		virtual bool Init(const void* pData, uint32 width, uint32 height) = 0;

		virtual uint32 GetWidth() const = 0;
		virtual uint32 GetHeight() const = 0;

		static ITexture2D* CreateTexture(const void* pData, uint32 width, uint32 height);
		static ITexture2D* CreateTexture(const String& file);
	};

} }
