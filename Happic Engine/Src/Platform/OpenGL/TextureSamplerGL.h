#pragma once

#include "../../Rendering/ITextureSampler.h"
#include <GL\glew.h>

namespace Happic { namespace Rendering {

	class TextureSamplerGL : public ITextureSampler
	{
	public:
		void Init(FilterType filter, AddressMode addressMode, AnisotropyLevel anisotropyLevel) override;

		void BindSampler() const;
	private:
		GLenum m_filter;
		GLenum m_wrap;
		AnisotropyLevel m_anisotropyLevel;
	};

} }
