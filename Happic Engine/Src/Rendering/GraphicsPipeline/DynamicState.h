#pragma once

namespace Happic { namespace Rendering {

	struct DynamicState
	{
		DynamicState(bool viewport = true, bool scissor = true, bool blendState = true) :
			viewport(viewport),
			scissor(scissor),
			blendState(blendState)
		{}

		bool viewport;
		bool scissor;
		bool blendState;
	};

} }
