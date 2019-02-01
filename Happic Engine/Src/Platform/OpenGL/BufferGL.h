#pragma once

#include "../../Rendering/IBuffer.h"
#include <GL\glew.h>

namespace Happic { namespace Rendering {

	class BufferGL : public IBuffer
	{
	public:
		BufferGL();
		~BufferGL();

		bool Init(BufferType type, BufferUsage usage, const void* pData, uint32 size) override;

		void* Map() const override;
		void Unmap() const override;

		uint32 GetSize() const override;

		GLuint GetVao() const;
		GLuint GetBuffer() const;
	private:
		GLuint m_vao;
		GLuint m_buffer;
		GLenum m_target;
		uint32 m_size;
	};

} }
