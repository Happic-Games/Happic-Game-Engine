#pragma once

#include "../../Rendering/ITexture2D.h"
#include <GL\glew.h>

namespace Happic { namespace Rendering {

	class Texture2DGL : public ITexture2D
	{
	public:
		bool Init(const void* pData, uint32 width, uint32 height) override;

		uint32 GetWidth() const override;
		uint32 GetHeight() const override;

		GLuint GetTexture() const;
	private:
		GLuint m_texture;
		uint32 m_width;
		uint32 m_height;
	};

} }
