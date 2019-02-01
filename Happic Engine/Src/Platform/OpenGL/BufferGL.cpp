#include "BufferGL.h"
#include "RenderContextGL.h"

namespace Happic { namespace Rendering {

	BufferGL::BufferGL()
	{
		m_vao = 0;
	}

	BufferGL::~BufferGL()
	{
		if(m_vao)
			glDeleteVertexArrays(1, &m_vao);
		glDeleteBuffers(1, &m_buffer);
	}

	bool BufferGL::Init(BufferType type, BufferUsage usage, const void * pData, uint32 size)
	{
		m_size = size;
		switch (type)
		{
		case BUFFER_TYPE_VERTEX: m_target = GL_ARRAY_BUFFER; break;
		case BUFFER_TYPE_INDEX: m_target = GL_ELEMENT_ARRAY_BUFFER; break;
		case BUFFER_TYPE_UNIFORM: m_target = GL_UNIFORM_BUFFER; break;
		}

		if (type == BUFFER_TYPE_VERTEX)
		{
			glGenVertexArrays(1, &m_vao);
			glBindVertexArray(m_vao);
		}

		glGenBuffers(1, &m_buffer);
		glBindBuffer(m_target, m_buffer);
		glBufferData(m_target, size, pData, usage == BUFFER_USAGE_DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

		if (type == BUFFER_TYPE_VERTEX)
		{
			const VertexInputLayout& layout = ((RenderContextGL*)IRenderContext::GetRenderContext())->GetInputLayout();
			for (uint32 i = 0; i < layout.numVertexAttributes; i++)
			{
				uint32 size = 0;

				switch (layout.vertexAttributes[i].type)
				{
				case VERTEX_ATTRIBUTE_FLOAT:  size = 1; break;
				case VERTEX_ATTRIBUTE_FLOAT2: size = 2; break;
				case VERTEX_ATTRIBUTE_FLOAT3: size = 3; break;
				case VERTEX_ATTRIBUTE_FLOAT4: size = 4; break;
				}

				glEnableVertexAttribArray(layout.vertexAttributes[i].location);
				glVertexAttribPointer(layout.vertexAttributes[i].location, size, GL_FLOAT, GL_FALSE, layout.vertexSize,
					(GLvoid*)layout.vertexAttributes[i].offset);

			}

			glBindVertexArray(0);
		}

		glBindBuffer(m_target, 0);

		return true;
	}

	void * BufferGL::Map() const
	{
		glBindBuffer(m_target, m_buffer);
		return glMapBuffer(m_target, GL_WRITE_ONLY);
	}

	void BufferGL::Unmap() const
	{
		glBindBuffer(m_target, m_buffer);
		glUnmapBuffer(m_target);
		glBindBuffer(m_target, 0);
	}

	uint32 BufferGL::GetSize() const
	{
		return m_size;
	}

	GLuint BufferGL::GetVao() const
	{
		return m_vao;
	}

	GLuint BufferGL::GetBuffer() const
	{
		return m_buffer;
	}

} }
