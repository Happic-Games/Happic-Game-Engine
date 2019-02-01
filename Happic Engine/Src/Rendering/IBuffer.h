#pragma once

#include <vector>
#include "Vertex.h"
#include "../DataStructures/Types.h"

namespace Happic { namespace Rendering {

	enum BufferType
	{
		BUFFER_TYPE_VERTEX,
		BUFFER_TYPE_INDEX,
		BUFFER_TYPE_UNIFORM,
		
		NUM_BUFFER_TYPES
	};

	enum BufferUsage
	{
		BUFFER_USAGE_STATIC,
		BUFFER_USAGE_DYNAMIC,
	};

	class IBuffer
	{
	public:
		virtual ~IBuffer() {}

		virtual bool Init(BufferType type, BufferUsage usage, const void* pData, uint32 size) = 0;

		virtual void* Map() const = 0;
		virtual void Unmap() const = 0;

		virtual uint32 GetSize() const = 0;
	
		static IBuffer* CreateBuffer(BufferType type, BufferUsage usage, const void* pData, uint32 size);
	};

} }
