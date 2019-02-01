#include "BufferVK.h"
#include "RenderContextVK.h"

namespace Happic { namespace Rendering {

	BufferVK::BufferVK()
	{
		
	}

	BufferVK::~BufferVK()
	{
		const VkDevice& device = ((RenderContextVK*)IRenderContext::GetRenderContext())->GetDevice();
		vkDestroyBuffer(device, m_buffer, NULL);
		vkFreeMemory(device, m_bufferMemory, NULL);
	}

	bool BufferVK::Init(BufferType type, BufferUsage usage, const void * pData, uint32 size)
	{
		m_size = size;
		RenderContextVK* pRenderContext = (RenderContextVK*)IRenderContext::GetRenderContext();
		const VkDevice& device = pRenderContext->GetDevice();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VkBufferUsageFlags buffer_usage_flags;
		if (usage == BUFFER_USAGE_DYNAMIC)
		{
			switch (type)
			{
			case BUFFER_TYPE_VERTEX: buffer_usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; break;
			case BUFFER_TYPE_INDEX: buffer_usage_flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT; break;
			case BUFFER_TYPE_UNIFORM: buffer_usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
			}
		}
		else if (usage == BUFFER_USAGE_STATIC)
		{
			buffer_usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}

		bool success = CreateBuffer(size, buffer_usage_flags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&stagingBuffer, &stagingBufferMemory);

		if (!success)
			return false;

		if (pData)
		{
			void* pMemory;
			VkResult err = vkMapMemory(device, stagingBufferMemory, 0, size, 0, &pMemory);

			if (err != VK_SUCCESS)
			{
				std::cerr << "Error mapping Vulkan buffer memory" << std::endl;
				return false;
			}

			memcpy(pMemory, pData, size);
			vkUnmapMemory(device, stagingBufferMemory);
		}

		if (usage == BUFFER_USAGE_DYNAMIC)
		{
			m_buffer = stagingBuffer;
			m_bufferMemory = stagingBufferMemory;
		}
		else if (usage == BUFFER_USAGE_STATIC)
		{
			VkBufferUsageFlags buffer_usage_flags_static;

			switch (type)
			{
			case BUFFER_TYPE_VERTEX: buffer_usage_flags_static = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; break;
			case BUFFER_TYPE_INDEX: buffer_usage_flags_static = VK_BUFFER_USAGE_INDEX_BUFFER_BIT; break;
			case BUFFER_TYPE_UNIFORM: buffer_usage_flags_static = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
			}

			success = CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | buffer_usage_flags_static, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_buffer, &m_bufferMemory);

			if (!success)
				return false;

			pRenderContext->CopyBuffer(stagingBuffer, m_buffer, size);
			vkDestroyBuffer(device, stagingBuffer, NULL);
			vkFreeMemory(device, stagingBufferMemory, NULL);
		}

		return true;
	}

	void * BufferVK::Map() const
	{
		void* pData;
		VkResult err = vkMapMemory(((RenderContextVK*)IRenderContext::GetRenderContext())->GetDevice(), m_bufferMemory, 0, m_size, 0, &pData);
		
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error mapping Vulkan buffer" << std::endl;
			return NULL;
		}

		return pData;
	}

	void BufferVK::Unmap() const
	{
		vkUnmapMemory(((RenderContextVK*)IRenderContext::GetRenderContext())->GetDevice(), m_bufferMemory);
	}

	uint32 BufferVK::GetSize() const
	{
		return m_size;
	}

	const VkBuffer & BufferVK::GetBuffer() const
	{
		return m_buffer;
	}

	bool BufferVK::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer * pBuffer, VkDeviceMemory * pBufferMemory)
	{
		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = size;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		buffer_create_info.usage = usage;


		RenderContextVK* pRenderContext = (RenderContextVK*)IRenderContext::GetRenderContext();
		const VkDevice& device = pRenderContext->GetDevice();

		VkResult err = vkCreateBuffer(device, &buffer_create_info, NULL, pBuffer);
		if (err != VK_SUCCESS)
		{
			std::cout << "Error creating Vulkan buffer" << std::endl;
			return false;
		}

		VkMemoryRequirements memory_requirements{};
		vkGetBufferMemoryRequirements(device, *pBuffer, &memory_requirements);

		VkMemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_allocate_info.memoryTypeIndex = pRenderContext->FindMemoryType(memory_requirements.memoryTypeBits, properties);

		err = vkAllocateMemory(device, &memory_allocate_info, NULL, pBufferMemory);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error allocation buffer memory" << std::endl;
			return false;
		}

		err = vkBindBufferMemory(device, *pBuffer, *pBufferMemory, 0);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error binding memory to Vulkan buffer" << std::endl;
			return false;
		}

		return true;
	}

} }

