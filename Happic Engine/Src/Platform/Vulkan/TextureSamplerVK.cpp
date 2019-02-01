#include "TextureSamplerVK.h"
#include "RenderContextVK.h"

namespace Happic { namespace Rendering {

	TextureSamplerVK::~TextureSamplerVK()
	{
		vkDestroySampler(((RenderContextVK*)IRenderContext::GetRenderContext())->GetDevice(), m_sampler, NULL);
	}

	void TextureSamplerVK::Init(FilterType filter, AddressMode addressMode, AnisotropyLevel anisotropyLevel)
	{
		VkSamplerCreateInfo sampler_create_info {};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		VkSamplerAddressMode sampler_address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		switch (addressMode)
		{
		case ADDRESS_MODE_CLAMP_TO_BORDER: sampler_address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; break;
		case ADDRESS_MODE_CLAMP_TO_EDGE: sampler_address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; break;
		case ADDRESS_MODE_REPEAT: sampler_address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT; break;
		case ADDRESS_MODE_MIRRORED_REPEAT: sampler_address_mode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; break;
		}

		VkFilter filterVK;
		switch (filter)
		{
		case FILTER_NEAREST: filterVK = VK_FILTER_NEAREST; break;
		case FILTER_LINEAR: filterVK = VK_FILTER_LINEAR; break;
		}

		sampler_create_info.minFilter = filterVK;
		sampler_create_info.magFilter = filterVK;
		sampler_create_info.addressModeU = sampler_address_mode;
		sampler_create_info.addressModeV = sampler_address_mode;
		sampler_create_info.addressModeW = sampler_address_mode;
		sampler_create_info.anisotropyEnable = anisotropyLevel == ANISOTROPY_DISABLED ? VK_FALSE : VK_TRUE;
		sampler_create_info.maxAnisotropy = anisotropyLevel;
		sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_create_info.unnormalizedCoordinates = VK_FALSE;
		sampler_create_info.compareEnable = VK_FALSE;
		sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
		sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_create_info.mipLodBias = 0.0f;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = 0.0f;

		VkResult err = vkCreateSampler(((RenderContextVK*)IRenderContext::GetRenderContext())->GetDevice(),
			&sampler_create_info, NULL, &m_sampler);

		if (err != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan sampler" << std::endl;
			return;
		}
	}

	VkSampler TextureSamplerVK::GetSampler() const
	{
		return m_sampler;
	}

} }
