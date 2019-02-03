#pragma once

#include "../../Rendering/ITexture2D.h"
#include <vulkan\vulkan.h>

namespace Happic { namespace Rendering {

	class Texture2DVK : public ITexture2D
	{
	public:
		Texture2DVK();
		~Texture2DVK();

		bool Init(const void* pData, uint32 width, uint32 height) override;

		VkImageView GetImageView() const;

		uint32 GetWidth() const override;
		uint32 GetHeight() const override;
	private:
		static VkFormat GetFormat(TextureFormat format);
	private:
		VkImage				m_image;
		VkImageView			m_imageView;
		VkDeviceMemory		m_imageMemory;
		uint32				m_width;
		uint32				m_height;
	};

} }
