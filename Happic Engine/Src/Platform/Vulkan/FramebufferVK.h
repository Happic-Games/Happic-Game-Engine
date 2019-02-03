#pragma once

#include "../../Rendering/IFramebuffer.h"
#include <vulkan\vulkan.h>
#include "RenderContextVK.h"


namespace Happic { namespace Rendering {

	struct FramebufferAttachmentVK
	{
		VkImage				image;
		VkDeviceMemory		imageMemory;
		VkImageView			imageView;
		VkFormat			format;
	};

	class FramebufferVK : public IFramebuffer
	{
	public:
		FramebufferVK(uint32 width, uint32 height);
		~FramebufferVK();

		void AddRenderTexture(TextureFormat format) override;
		void AddDepthTexture(TextureFormat format) override;
		void Create() override;

		void PrepareForReading(bool readDepthBuffer) const override;
		void PrepareForRendering() const override;

		uint32 GetWidth() const override;
		uint32 GetHeight() const override;

		bool HasDepth() const override;

		VkFramebuffer GetFramebuffer() const;
		uint32 GetTextureAttachmentCount() const;
		VkImageView	GetRenderTexture(uint32 binding) const;

		VkImageView GetDepthTexture() const;
	private:
		static VkFormat GetFormat(TextureFormat format);
	private:
		RenderContextVK*			m_pRenderContext;

		VkFramebuffer				m_framebuffer;
		FramebufferAttachmentVK		m_attachments[FRAMEBUFFER_MAX_RENDER_TEXTURES];
		uint32						m_attachmentCount;
		FramebufferAttachmentVK		m_depthAttachment;
		bool						m_hasDepth;

		mutable VkImageLayout		m_currentColorLayout;
		mutable VkImageLayout		m_currentDepthLayout;

		uint32						m_width;
		uint32						m_height;
	};

} }
