#pragma once

#include "../../DataStructures/Types.h"
#include "../../DataStructures/String.h"
#define MAX_VERTEX_ATTRIBUTES 16

namespace Happic { namespace Rendering {

	enum VertexAttributeType
	{
		VERTEX_ATTRIBUTE_FLOAT,
		VERTEX_ATTRIBUTE_FLOAT2,
		VERTEX_ATTRIBUTE_FLOAT3,
		VERTEX_ATTRIBUTE_FLOAT4,
	};

	struct VertexAttribute
	{
		VertexAttribute(uint32 location, uint32 offset, VertexAttributeType type, const String& name) :
			location(location),
			offset(offset),
			type(type),
			name(name)
		{}

		VertexAttribute() {}

		uint32					location;
		uint32					offset;
		VertexAttributeType		type;
		String					name;
	};

	struct VertexInputLayout
	{
		uint32					vertexSize;
		uint32					numVertexAttributes;
		VertexAttribute			vertexAttributes[MAX_VERTEX_ATTRIBUTES];
	};

} }
