#include "BufferD3D11.h"
#include "RenderContextD3D11.h"

namespace Happic { namespace Rendering {

	BufferD3D11::BufferD3D11()
	{
	}

	BufferD3D11::~BufferD3D11()
	{
		m_pBuffer->Release();
	}

	bool BufferD3D11::Init(BufferType type, BufferUsage usage, const void * pData, uint32 size)
	{
		m_size = size;
		D3D11_BUFFER_DESC buffer_desc {};
		buffer_desc.ByteWidth = size;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;

		if (usage == BUFFER_USAGE_STATIC)
		{
			buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
			buffer_desc.CPUAccessFlags = 0;
		}
		else if (usage == BUFFER_USAGE_DYNAMIC)
		{
			buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
			buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}

		switch (type)
		{
		case BUFFER_TYPE_VERTEX: buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; break;
		case BUFFER_TYPE_INDEX: buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER; break;
		case BUFFER_TYPE_UNIFORM: buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; break;
		}

		HRESULT err = -1;
		if (pData)
		{
			D3D11_SUBRESOURCE_DATA init_data{};
			init_data.pSysMem = pData;
			init_data.SysMemPitch = 0;
			init_data.SysMemSlicePitch;

			err = ((RenderContextD3D11*)IRenderContext::GetRenderContext())->GetDevice()->CreateBuffer(&buffer_desc, &init_data, &m_pBuffer);
			if (err != S_OK)
			{
				std::cerr << "Error creating D3D11 buffer" << std::endl;
				return false;
			}
		}
		else
		{
			err = ((RenderContextD3D11*)IRenderContext::GetRenderContext())->GetDevice()->CreateBuffer(&buffer_desc, NULL, &m_pBuffer);
		}

		if (err != S_OK)
		{
			std::cerr << "Error creating D3D11 buffer" << std::endl;
			return false;
		}

		return true;
	}

	void * BufferD3D11::Map() const
	{
		D3D11_MAPPED_SUBRESOURCE mapped_data {};
		HRESULT err = ((RenderContextD3D11*)IRenderContext::GetRenderContext())->GetDeviceContext()->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
		
		if (err != S_OK)
		{
			std::cerr << "Error mapping D3D11 buffer" << std::endl;
			return NULL;
		
		}
		return mapped_data.pData;
	}

	void BufferD3D11::Unmap() const
	{
		((RenderContextD3D11*)IRenderContext::GetRenderContext())->GetDeviceContext()->Unmap(m_pBuffer, 0);
	}

	uint32 BufferD3D11::GetSize() const
	{
		return m_size;
	}

	ID3D11Buffer * BufferD3D11::GetBuffer()
	{
		return m_pBuffer;
	}

} }
