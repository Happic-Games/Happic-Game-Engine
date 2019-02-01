#include "TextureSamplerD3D11.h"
#include "RenderContextD3D11.h"

namespace Happic { namespace Rendering {

	TextureSamplerD3D11::~TextureSamplerD3D11()
	{
		m_pSamplerState->Release();
	}

	void TextureSamplerD3D11::Init(FilterType filter, AddressMode addressMode, AnisotropyLevel anisotropyLevel)
	{
		D3D11_SAMPLER_DESC sampler_desc {};

		D3D11_FILTER filterD3D11 = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		D3D11_TEXTURE_ADDRESS_MODE addressModeD3D11 = D3D11_TEXTURE_ADDRESS_WRAP;

		switch (filter)
		{
		case FILTER_LINEAR: filterD3D11 = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
		case FILTER_NEAREST: filterD3D11 = D3D11_FILTER_MIN_MAG_MIP_POINT; break;
		}

		switch (addressMode)
		{
		case ADDRESS_MODE_CLAMP_TO_BORDER: addressModeD3D11 = D3D11_TEXTURE_ADDRESS_BORDER; break;
		case ADDRESS_MODE_CLAMP_TO_EDGE:  addressModeD3D11 = D3D11_TEXTURE_ADDRESS_CLAMP; break;
		case ADDRESS_MODE_MIRRORED_REPEAT: addressModeD3D11 = D3D11_TEXTURE_ADDRESS_MIRROR; break;
		case ADDRESS_MODE_REPEAT: addressModeD3D11 = D3D11_TEXTURE_ADDRESS_WRAP; break;
		}

		sampler_desc.Filter = filterD3D11;
		sampler_desc.AddressU = addressModeD3D11;
		sampler_desc.AddressV = addressModeD3D11;
		sampler_desc.AddressW = addressModeD3D11;
		sampler_desc.MipLODBias = 0.0f;
		sampler_desc.MaxAnisotropy = anisotropyLevel;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampler_desc.BorderColor[0] = 0;
		sampler_desc.BorderColor[1] = 0;
		sampler_desc.BorderColor[2] = 0;
		sampler_desc.BorderColor[3] = 0;
		sampler_desc.MinLOD = 0;
		sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

		HRESULT err = ((RenderContextD3D11*)IRenderContext::GetRenderContext())->GetDevice()->CreateSamplerState(&sampler_desc, &m_pSamplerState);
		if (err != S_OK)
		{
			std::cerr << "Error creating D3D11 sampler" << std::endl;
			return;
		}
	}

	ID3D11SamplerState * TextureSamplerD3D11::GetSamplerState() const
	{
		return m_pSamplerState;
	}

} }
