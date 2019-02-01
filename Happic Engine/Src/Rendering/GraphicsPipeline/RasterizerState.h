#pragma once

namespace Happic { namespace Rendering {

	enum PolygonMode
	{
		POLYGON_MODE_FILL,
		POLYGON_MODE_LINE,
		POLYGON_MODE_POINT,

		NUM_POLYGON_MODES
	};

	enum CullMode
	{
		CULL_MODE_FRONT,
		CULL_MODE_BACK,
		CULL_MODE_NONE,
		CULL_MODE_FRONT_BACK,

		NUM_CULL_MODES
	};

	enum FrontFace
	{
		FRONT_FACE_CW,
		FRONT_FACE_CCW
	};

	struct RasterizerState
	{
		RasterizerState(PolygonMode polygonMode = POLYGON_MODE_FILL, CullMode cullMode = CULL_MODE_BACK, FrontFace frontFace = FRONT_FACE_CW) :
			polygonMode(polygonMode),
			cullMode(cullMode),
			frontFace(frontFace)
		{}

		PolygonMode polygonMode;
		CullMode	cullMode;
		FrontFace	frontFace;
	};

} }
