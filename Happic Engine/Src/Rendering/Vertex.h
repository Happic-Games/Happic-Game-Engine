#pragma once

#include "../Math/Math.h"

namespace Happic { namespace Rendering {

	struct Vertex
	{
		Vertex(const Math::Vector3f& pos, const Math::Vector2f& texCoord) :
			pos(pos),
			texCoord(texCoord)
		{}

		Vertex() {}

		Math::Vector3f pos;
		Math::Vector2f texCoord;
	};

} }
