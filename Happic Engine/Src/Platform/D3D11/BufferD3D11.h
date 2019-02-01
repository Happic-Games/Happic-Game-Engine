#pragma once

#include "../../Rendering/IBuffer.h"
#include <d3d11.h>

namespace Happic { namespace Rendering {

	class BufferD3D11 : public IBuffer
	{
	public:
		BufferD3D11();
		~BufferD3D11();

		bool Init(BufferType type, BufferUsage usage, const void* pData, uint32 size) override;

		void* Map() const override;
		void Unmap() const override;

		uint32 GetSize() const override;

		ID3D11Buffer* GetBuffer();
	private:
		ID3D11Buffer* m_pBuffer;
		uint32 m_size;
	};

} }
