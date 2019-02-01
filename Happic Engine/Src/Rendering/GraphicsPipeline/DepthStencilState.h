#pragma once

namespace Happic { namespace Rendering {

	enum Comparison
	{
		COMPARISON_LESS,
		COMPARISON_GREATER,
		COMPARISON_LESS_EQUAL,
		COMPARISON_GREATER_EQUAL,
		COMPARISON_EQUAL,
		COMPARISON_NOT_EQUAL,
		COMPARISON_NEVER,
		COMPARISON_ALWAYS,
	};

	enum StencilOperation
	{
		STENCIL_OPERATION_KEEP,
		STENCIL_OPERATION_ZERO,
		STENCIL_OPERATION_REPLACE,
		STENCIL_OPERATION_INCR_CLAMP,
		STENCIL_OPERATION_DECR_CLAMP,
		STENCIL_OPERATION_INVERT,
		STENCIL_OPERATION_INCR,
		STENCIL_OPERATION_DECR,

		NUM_STENCIL_OPERATIONS
	};

	struct StencilFace
	{
		StencilFace(StencilOperation stencilFail, StencilOperation depthStencilPass, StencilOperation depthStencilFailt, Comparison comparison) :
			stencilFail(stencilFail),
			depthStencilPass(depthStencilPass),
			depthStencilFail(depthStencilFail),
			comparison(comparison)
		{}

		StencilFace() {}

		StencilOperation	stencilFail;
		StencilOperation	depthStencilPass;
		StencilOperation	depthStencilFail;
		Comparison			comparison;
	};

	struct DepthStencilState
	{
		DepthStencilState(bool depthTestEnabled, bool depthWriteEnabled, Comparison depthComparison, bool stencilTestEnabled,
			const StencilFace& front, const StencilFace& back) :
			depthTestEnabled(depthTestEnabled),
			depthWriteEnabled(depthWriteEnabled),
			depthComparison(depthComparison),
			stencilTestEnabled(stencilTestEnabled),
			front(front),
			back(back)
		{}

		DepthStencilState() {}

		bool depthTestEnabled;
		bool depthWriteEnabled;
		Comparison depthComparison;
		bool stencilTestEnabled;
		StencilFace front;
		StencilFace back;
	};

} }
