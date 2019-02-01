#pragma once

namespace Happic { namespace Rendering {

	enum FilterType
	{
		FILTER_NEAREST,
		FILTER_LINEAR,
		NUM_FILTER_TYPES
	};

	enum AddressMode
	{
		ADDRESS_MODE_REPEAT,
		ADDRESS_MODE_MIRRORED_REPEAT,
		ADDRESS_MODE_CLAMP_TO_EDGE,
		ADDRESS_MODE_CLAMP_TO_BORDER,

	};

	enum AnisotropyLevel
	{
		ANISOTROPY_DISABLED				= 1,
		ANISOTROPY_2					= 2,
		ANISOTROPY_4					= 4,
		ANISOTROPY_8					= 8,
		ANISOTROPY_16					= 16,
		NUM_ANISOTROPY_LEVELS			= 5
	};

	class ITextureSampler
	{
	public:
		virtual ~ITextureSampler() {}
		virtual void Init(FilterType filterType, AddressMode addressMode, AnisotropyLevel anisotropyLevel) = 0;

		static ITextureSampler* CreateTextureSampler(FilterType filterType, AddressMode addressMode, AnisotropyLevel anisotropyLevel);
	};

} }
