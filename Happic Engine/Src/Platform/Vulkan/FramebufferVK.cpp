#include "FramebufferVK.h"

namespace Happic { namespace Rendering {

	FramebufferVK::FramebufferVK(uint32 width, uint32 height) :
		m_width(width),
		m_height(height),
		m_attachmentCount(0),
		m_hasDepth(false)
	{
		m_pRenderContext = (RenderContextVK*)IRenderContext::GetRenderContext();
	}

	FramebufferVK::~FramebufferVK()
	{
		const VkDevice& device = m_pRenderContext->GetDevice();

		for (uint32 i = 0; i < m_attachmentCount; i++)
		{
			vkDestroyImage(device, m_attachments[i].image, NULL);
			vkDestroyImageView(device, m_attachments[i].imageView, NULL);
			vkFreeMemory(device, m_attachments[i].imageMemory, NULL);
		}

		vkDestroyImage(device, m_depthAttachment.image, NULL);
		vkDestroyImageView(device, m_depthAttachment.imageView, NULL);
		vkFreeMemory(device, m_depthAttachment.imageMemory, NULL);

		vkDestroyFramebuffer(device, m_framebuffer, NULL);
	}

	void FramebufferVK::AddRenderTexture(TextureFormat format)
	{
		if (m_attachmentCount >= FRAMEBUFFER_MAX_RENDER_TEXTURES)
		{
			std::cerr << "The max framebuffer attachments is :" << (FRAMEBUFFER_MAX_RENDER_TEXTURES - 1) << std::endl;
			return;
		}

		VkFormat formatVK = GetFormat(format);

		FramebufferAttachmentVK attachment;
		attachment.format = formatVK;

		bool success = m_pRenderContext->CreateImage(m_width, m_height, formatVK, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &attachment.image, &attachment.imageMemory);

		if (!success)
		{
			std::cerr << "Error creating Vulkan image" << std::endl;
			return;
		}

		m_pRenderContext->TransitionImageLayout(attachment.image, formatVK, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		m_currentColorLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		success = m_pRenderContext->CreateImageView(attachment.image, formatVK, VK_IMAGE_ASPECT_COLOR_BIT, &attachment.imageView);
		if (!success)
		{
			std::cerr << "Error creating Vulkan image view" << std::endl;
			return;
		}

		m_attachments[m_attachmentCount++] = attachment;
	}

	void FramebufferVK::AddDepthTexture(TextureFormat format)
	{
		if (m_hasDepth)
		{
			std::cerr << "Only one depth texture per framebuffer" << std::endl;
			return;
		}

		if (format != TEXTURE_FORMAT_DEPTH16_UNORM && format != TEXTURE_FORMAT_DEPTH16_UNORM_STENCIL8_UINT &&
			format != TEXTURE_FORMAT_DEPTH24_UNORM_STENCIL8_UINT && format != TEXTURE_FORMAT_DEPTH32_FLOAT && TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_UINT)
		{
			std::cerr << "Invalid depth texture format" << std::endl;
			return;
		}

		VkFormat formatVK = GetFormat(format);
		m_depthAttachment.format = formatVK;

		bool success = m_pRenderContext->CreateImage(m_width, m_height, formatVK, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_depthAttachment.image, &m_depthAttachment.imageMemory);

		if (!success)
		{
			std::cerr << "Error creating Vulkan image" << std::endl;
			return;
		}

		m_pRenderContext->TransitionImageLayout(m_depthAttachment.image, formatVK, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		m_currentDepthLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkImageAspectFlags image_aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (format == TEXTURE_FORMAT_DEPTH16_UNORM_STENCIL8_UINT || format == TEXTURE_FORMAT_DEPTH24_UNORM_STENCIL8_UINT ||
			format == TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_UINT)
		{
			image_aspect_flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		success = m_pRenderContext->CreateImageView(m_depthAttachment.imageView, formatVK, image_aspect_flags, &m_depthAttachment.imageView);

		m_hasDepth = true;
	}

	void FramebufferVK::Create()
	{
		std::vector<VkImageView> attachments;
		for (uint32 i = 0; i < m_attachmentCount; i++)
			attachments.push_back(m_attachments[i].imageView);

		uint32 attachmentCount = m_attachmentCount;
		if (m_hasDepth)
		{
			attachmentCount++;
			attachments.push_back(m_depthAttachment.imageView);
		}

		VkFramebufferCreateInfo framebuffer_create_info {};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.attachmentCount = attachmentCount;
		framebuffer_create_info.pAttachments = &attachments[0];
		framebuffer_create_info.width = m_width;
		framebuffer_create_info.height = m_height;
		framebuffer_create_info.renderPass = m_pRenderContext->GetRenderPass();
		framebuffer_create_info.layers = 1;
		
		VkResult err = vkCreateFramebuffer(m_pRenderContext->GetDevice(), &framebuffer_create_info, NULL, &m_framebuffer);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan framebuffer" << std::endl;
			return;
		}
	}

	void FramebufferVK::PrepareForReading(bool readDepthBuffer) const
	{
		if (m_currentColorLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			for (uint32 i = 0; i < m_attachmentCount; i++)
				m_pRenderContext->TransitionImageLayout(m_attachments[i].image, m_attachments[i].format, m_currentColorLayout,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		if (readDepthBuffer && m_currentDepthLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			m_pRenderContext->TransitionImageLayout(m_depthAttachment.image, m_depthAttachment.format, m_currentDepthLayout,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_currentColorLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_currentDepthLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	void FramebufferVK::PrepareForRendering() const
	{
		if (m_currentColorLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			for (uint32 i = 0; i < m_attachmentCount; i++)
				m_pRenderContext->TransitionImageLayout(m_attachments[i].image, m_attachments[i].format, m_currentColorLayout,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		if(m_currentDepthLayout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			m_pRenderContext->TransitionImageLayout(m_depthAttachment.image, m_depthAttachment.format, m_currentDepthLayout,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		m_currentColorLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		m_currentDepthLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	uint32 FramebufferVK::GetWidth() const
	{
		return m_width;
	}

	uint32 FramebufferVK::GetHeight() const
	{
		return m_height;
	}

	bool FramebufferVK::HasDepth() const
	{
		return m_hasDepth;
	}

	VkFramebuffer FramebufferVK::GetFramebuffer() const
	{
		return m_framebuffer;
	}

	uint32 FramebufferVK::GetTextureAttachmentCount() const
	{
		return m_attachmentCount;
	}

	VkImageView FramebufferVK::GetRenderTexture(uint32 binding) const
	{
		return m_attachments[binding].imageView;
	}

	VkImageView FramebufferVK::GetDepthTexture() const
	{
		return m_depthAttachment.imageView;
	}

	VkFormat FramebufferVK::GetFormat(TextureFormat format)
	{
		switch (format)
		{
		case TEXTURE_FORMAT_RGBA8_UNORM:					return VK_FORMAT_R8G8B8A8_UNORM;
		case TEXTURE_FORMAT_RGBA16_UNORM:					return VK_FORMAT_R16G16B16A16_UNORM;
		case TEXTURE_FORMAT_RGBA16_FLOAT:					return VK_FORMAT_R16G16B16A16_SFLOAT;
		case TEXTURE_FORMAT_RGBA32_FLOAT:					return VK_FORMAT_R32G32B32A32_SFLOAT;
		case TEXTURE_FORMAT_RGB8_UNORM:						return VK_FORMAT_R8G8B8_UNORM;
		case TEXTURE_FORMAT_RGB16_UNORM:					return VK_FORMAT_R16G16B16_UNORM;
		case TEXTURE_FORMAT_RGB16_FLOAT:					return VK_FORMAT_R16G16B16_SFLOAT;
		case TEXTURE_FORMAT_RGB32_FLOAT:					return VK_FORMAT_R32G32B32_SFLOAT;
		case TEXTURE_FORMAT_RG8_UNORM:						return VK_FORMAT_R8G8_UNORM;
		case TEXTURE_FORMAT_RG16_UNORM:						return VK_FORMAT_R16G16_UNORM;
		case TEXTURE_FORMAT_RG16_FLOAT:						return VK_FORMAT_R16G16_SFLOAT;
		case TEXTURE_FORMAT_RG32_FLOAT:						return VK_FORMAT_R32G32_SFLOAT;
		case TEXTURE_FORMAT_R8_UNORM:						return VK_FORMAT_R8_UNORM;
		case TEXTURE_FORMAT_R16_UNORM:						return VK_FORMAT_R16_UNORM;
		case TEXTURE_FORMAT_R16_FLOAT:						return VK_FORMAT_R16_SFLOAT;
		case TEXTURE_FORMAT_R32_FLOAT:						return VK_FORMAT_R32_SFLOAT;
		case TEXTURE_FORMAT_DEPTH16_UNORM:					return VK_FORMAT_D16_UNORM;
		case TEXTURE_FORMAT_DEPTH16_UNORM_STENCIL8_UINT:	return VK_FORMAT_D16_UNORM_S8_UINT;
		case TEXTURE_FORMAT_DEPTH24_UNORM_STENCIL8_UINT:	return VK_FORMAT_D24_UNORM_S8_UINT;
		case TEXTURE_FORMAT_DEPTH32_FLOAT:					return VK_FORMAT_D32_SFLOAT;
		case TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_UINT:	return VK_FORMAT_D32_SFLOAT_S8_UINT;
		}
	}

} }
