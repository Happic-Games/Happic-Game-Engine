#include "Texture2DVK.h"
#include "BufferVK.h"
#include "RenderContextVK.h"

namespace Happic { namespace Rendering {

	Texture2DVK::Texture2DVK()
	{
	}

	Texture2DVK::~Texture2DVK()
	{
		const VkDevice& device = ((RenderContextVK*)IRenderContext::GetRenderContext())->GetDevice();
		vkDestroyImage(device, m_image, NULL);
		vkFreeMemory(device, m_imageMemory, NULL);
		vkDestroyImageView(device, m_imageView, NULL);
	}

	bool Texture2DVK::Init(const void * pData, uint32 width, uint32 height)
	{
		m_width = width;
		m_height = height;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		RenderContextVK* pRenderContext = (RenderContextVK*)IRenderContext::GetRenderContext();
		const VkDevice& device = pRenderContext->GetDevice();

		uint32 bufferSize = width * height * 4;
		BufferVK::CreateBuffer(width * height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

		void* pBufferData = NULL;
		VkResult err = vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &pBufferData);

		if (err != VK_SUCCESS)
		{
			std::cerr << "Error mapping Vulkan buffer" << std::endl;
			return false;
		}

		memcpy(pBufferData, pData, bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		bool success = pRenderContext->CreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_image, &m_imageMemory);

		if (!success)
			return false;

		pRenderContext->TransitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		pRenderContext->CopyBufferToImage(stagingBuffer, m_image, width, height);
		pRenderContext->TransitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device, stagingBuffer, NULL);
		vkFreeMemory(device, stagingBufferMemory, NULL);

		success = pRenderContext->CreateImageView(m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &m_imageView);

		if (!success)
			return false;

		return true;
	}

	VkImageView Texture2DVK::GetImageView() const
	{
		return m_imageView;
	}

	uint32 Texture2DVK::GetWidth() const
	{
		return m_width;
	}

	uint32 Texture2DVK::GetHeight() const
	{
		return m_height;
	}

	VkFormat Texture2DVK::GetFormat(TextureFormat format)
	{
		return VkFormat();
	}

} }
