#pragma once

#include "../../Rendering/ITextureSampler.h"
#include <vulkan\vulkan.h>

namespace Happic { namespace Rendering {

	class TextureSamplerVK : public ITextureSampler
	{
	public:
		~TextureSamplerVK();
		void Init(FilterType filter, AddressMode addressMode, AnisotropyLevel anisotropyLevel);

		VkSampler GetSampler() const;
	private:
		VkSampler m_sampler;
	};

} }
