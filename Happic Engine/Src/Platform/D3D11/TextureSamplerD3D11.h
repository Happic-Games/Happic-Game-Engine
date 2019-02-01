#pragma once

#include "./../../Rendering/ITextureSampler.h"
#include <d3d11.h>

namespace Happic { namespace Rendering {

	class TextureSamplerD3D11 : public ITextureSampler
	{
	public:
		~TextureSamplerD3D11();

		void Init(FilterType filter, AddressMode addressMode, AnisotropyLevel anisotropyLevel);

		ID3D11SamplerState* GetSamplerState() const;
	private:
		ID3D11SamplerState* m_pSamplerState;
	};

} }
