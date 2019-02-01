#pragma once

#include "../../Rendering/ITexture2D.h"
#include <d3d11.h>

namespace Happic { namespace Rendering {

	class Texture2DD3D11 : public ITexture2D
	{
	public:
		~Texture2DD3D11();
		bool Init(const void* pData, uint32 width, uint32 height) override;

		ID3D11ShaderResourceView* GetShaderResourceView() const;

		uint32 GetWidth() const override;
		uint32 GetHeight() const override;
	private:
		ID3D11Texture2D*			m_pTexture;
		ID3D11ShaderResourceView*	m_pShaderResourceView;
		uint32 m_width;
		uint32 m_height;
	};

} }
