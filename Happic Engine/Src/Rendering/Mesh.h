#pragma once

#include "IBuffer.h"
#include "Material.h"
#include "DrawCommand.h"

namespace Happic { namespace Rendering {

	struct Mesh : public DrawCommand
	{
	public:
		Mesh(const Vertex* pVertices, uint32 numVertices, const uint32* pIndices, uint32 numIndices, Material material);
		Mesh() {}
	};

} }
