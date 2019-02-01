#include "TextureSamplerGL.h"

namespace Happic { namespace Rendering {

	void TextureSamplerGL::Init(FilterType filter, AddressMode addressMode, AnisotropyLevel anisotropyLevel)
	{
		switch (filter)
		{
		case FILTER_LINEAR: m_filter = GL_LINEAR; break;
		case FILTER_NEAREST: m_filter = GL_NEAREST; break;
		}

		switch (addressMode)
		{
		case ADDRESS_MODE_CLAMP_TO_BORDER: m_wrap = GL_CLAMP_TO_BORDER; break;
		case ADDRESS_MODE_CLAMP_TO_EDGE: m_wrap = GL_CLAMP_TO_EDGE; break;
		case ADDRESS_MODE_MIRRORED_REPEAT: m_wrap = GL_MIRRORED_REPEAT; break;
		case ADDRESS_MODE_REPEAT: m_wrap = GL_REPEAT; break;
		}

		m_anisotropyLevel = anisotropyLevel;
	}

	void TextureSamplerGL::BindSampler() const
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter);
	}


} }