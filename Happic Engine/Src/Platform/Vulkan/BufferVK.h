#pragma once

#include "../../Rendering/IBuffer.h"
#include <vulkan\vulkan.h>

namespace Happic { namespace Rendering {

	class BufferVK : public IBuffer
	{
	public:
		BufferVK();
		~BufferVK();

		bool Init(BufferType type, BufferUsage usage, const void* pData, uint32 size) override;

		void* Map() const override;
		void Unmap() const override;

		uint32 GetSize() const override;

		const VkBuffer& GetBuffer() const;
	public:
		static bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer* pBuffer, VkDeviceMemory* pBufferMemory);
	private:
		VkBuffer		m_buffer;
		VkDeviceMemory	m_bufferMemory;
		uint32			m_size;
	};

} }
