#pragma once

namespace Happic { namespace Rendering {

	enum BlendOperation
	{
		BLEND_OPERATION_ADD,
		BLEND_OPERATION_SUB,
		BLEND_OPERATION_REV_SUB,
		BLEND_OPERATION_MIN,
		BLEND_OPERATION_MAX,

		NUM_BLEND_OPERATIONS
	};

	enum BlendFactor
	{
		BLEND_FACTOR_ZERO,
		BLEND_FACTOR_ONE,
		BLEND_FACTOR_SRC_COLOR,
		BLEND_FACTOR_INV_SOURCE_COLOR,
		BLEND_FACTOR_DEST_COLOR,
		BLEND_FACTOR_INV_DEST_COLOR,
		BLEND_FACTOR_SRC_ALPHA,
		BLEND_FACTOR_SRC_INV_ALPHA,
		BLEND_FACTOR_DEST_ALPHA,
		BLEND_FACTOR_DEST_INV_ALPHA,
	};

	struct ColorBlendState
	{
		ColorBlendState(bool enabled, BlendOperation operation, BlendFactor srcFactor, BlendFactor destFactor,
			BlendOperation alphaBlendOperation, BlendFactor alphaSrcFactor, BlendFactor alphaDestFactor) :
			enabled(enabled),
			operation(operation),
			srcFactor(srcFactor),
			destFactor(destFactor),
			alphaBlendOperation(alphaBlendOperation),
			alphaSrcFactor(alphaSrcFactor),
			alphaDestFactor(alphaDestFactor)
		{}

		ColorBlendState() {}

		bool			enabled;
		BlendOperation	operation;
		BlendFactor	srcFactor;
		BlendFactor	destFactor;
		BlendOperation	alphaBlendOperation;
		BlendFactor	alphaSrcFactor;
		BlendFactor	alphaDestFactor;
	};

} }
