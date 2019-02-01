#include "Mesh.h"

namespace Happic { namespace Rendering {

	Mesh::Mesh(const Vertex * pVertices, uint32 numVertices, const uint32 * pIndices, uint32 numIndices, Material material) 
	{
		type = DRAW_COMMAND_DRAW_INDEXED;
		pVertexBuffer = IBuffer::CreateBuffer(BUFFER_TYPE_VERTEX, BUFFER_USAGE_STATIC, pVertices, sizeof(Vertex) * numVertices);
		pIndexBuffer = IBuffer::CreateBuffer(BUFFER_TYPE_INDEX, BUFFER_USAGE_STATIC, pIndices, sizeof(uint32) * numIndices);
		AddTextureGroup(material);
		AddSubDraw(0, numIndices - 1, 0);
	}

} }
