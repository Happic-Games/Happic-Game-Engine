#include "Texture2DGL.h"

namespace Happic { namespace Rendering {

	bool Texture2DGL::Init(const void * pData, uint32 width, uint32 height)
	{
		m_width = width;
		m_height = height;
		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		return true;
	}

	uint32 Texture2DGL::GetWidth() const
	{
		return m_width;
	}

	uint32 Texture2DGL::GetHeight() const
	{
		return m_height;
	}

	GLuint Texture2DGL::GetTexture() const
	{
		return m_texture;
	}

} }