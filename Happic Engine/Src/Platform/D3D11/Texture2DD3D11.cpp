#include "Texture2DD3D11.h"
#include "RenderContextD3D11.h"

namespace Happic { namespace Rendering {

	Texture2DD3D11::~Texture2DD3D11()
	{
		m_pTexture->Release();
		m_pShaderResourceView->Release();
	}

	bool Texture2DD3D11::Init(const void * pData, uint32 width, uint32 height)
	{
		m_width = width;
		m_height = height;

		RenderContextD3D11* pRenderContext = (RenderContextD3D11*)IRenderContext::GetRenderContext();

		D3D11_TEXTURE2D_DESC texture_desc{};
		texture_desc.Width = width;
		texture_desc.Height = height;
		texture_desc.MipLevels = 1;
		texture_desc.ArraySize = 1;
		texture_desc.SampleDesc.Count = 1;
		texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texture_desc.MiscFlags = 0;
		texture_desc.Usage = D3D11_USAGE_DEFAULT;
		texture_desc.CPUAccessFlags = 0;

		HRESULT err;

		if (pData)
		{
			D3D11_SUBRESOURCE_DATA init_data{};
			init_data.pSysMem = pData;
			init_data.SysMemPitch = m_width * 4 * sizeof(unsigned char);
			init_data.SysMemSlicePitch = 0;
			err = pRenderContext->GetDevice()->CreateTexture2D(&texture_desc, &init_data, &m_pTexture);
		}
		else
		{
			err = pRenderContext->GetDevice()->CreateTexture2D(&texture_desc, NULL, &m_pTexture);
		}

		if (err != S_OK)
		{
			std::cerr << "Error creating D3D11 texture" << std::endl;
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
		shader_resource_view_desc.Format = texture_desc.Format;
		shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
		shader_resource_view_desc.Texture2D.MipLevels = 1;

		//Reminder : update this when I implement render targets
		err = pRenderContext->GetDevice()->CreateShaderResourceView(m_pTexture, NULL, &m_pShaderResourceView);

		if (err != S_OK)
		{
			std::cerr << "Error creating D3D11 shader resource view" << std::endl;
			return false;
		}

		return true;
	}

	ID3D11ShaderResourceView * Texture2DD3D11::GetShaderResourceView() const
	{
		return m_pShaderResourceView;
	}

	uint32 Texture2DD3D11::GetWidth() const
	{
		return m_width;
	}

	uint32 Texture2DD3D11::GetHeight() const
	{
		return m_height;
	}

} }
