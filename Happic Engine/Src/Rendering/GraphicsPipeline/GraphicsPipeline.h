#pragma once

#include "../../Math/Math.h"
#include "VertexInputLayout.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "ColorBlendState.h"
#include "DynamicState.h"
#include "ShaderInfo.h"
#include "../Vertex.h"

namespace Happic { namespace Rendering { 

	struct Rect
	{
		Rect(float x = 0.0f , float y = 0.0f, float width = 0.0f, float height = 0.0f) :
			x(x),
			y(y),
			width(width),
			height(height)
		{}

		float x;
		float y;
		float width;
		float height;
	};

	typedef Rect Viewport;
	typedef Rect Scissor;

	enum PrimitiveTopology
	{
		PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,

		NUM_PRIMITIVE_TOPOLOGY_TYPES
	};


	struct GraphicsPipeline
	{
		GraphicsPipeline(const ShaderInfo& shaderInfo, const VertexInputLayout& vertexInputLayout, const Viewport& viewport, const Scissor& scissor,
			const RasterizerState& rasterizerState, const DepthStencilState& depthStancilState, const ColorBlendState& colorBlendState,
			const DynamicState& dynamicState, PrimitiveTopology primitiveTopology) :
			shaderInfo(shaderInfo),
			inputLayout(inputLayout),
			viewport(viewport),
			scissor(scissor),
			rasterizerState(rasterizerState),
			depthStencilState(depthStencilState),
			colorBlendState(colorBlendState),
			dynamicState(dynamicState),
			primitiveTopology(primitiveTopology)
		{}

		GraphicsPipeline() {}

		ShaderInfo			shaderInfo;
		VertexInputLayout	inputLayout;
		Viewport			viewport;
		Scissor				scissor;
		RasterizerState		rasterizerState;
		DepthStencilState	depthStencilState;
		ColorBlendState		colorBlendState;
		DynamicState		dynamicState;
		PrimitiveTopology	primitiveTopology;

		inline static GraphicsPipeline CreateDefaultGraphicsPipeline(uint32 width, uint32 height)
		{
			GraphicsPipeline pipeline;
			pipeline.viewport = Viewport(0, 0, width, height);
			pipeline.scissor = Scissor(0, 0, width, height);
			pipeline.primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			pipeline.rasterizerState = RasterizerState(POLYGON_MODE_FILL, CULL_MODE_NONE, FRONT_FACE_CCW);
			pipeline.shaderInfo = ShaderInfo("Default.vert", "Default.frag");
			pipeline.dynamicState = DynamicState(false, false, false);
			pipeline.colorBlendState = ColorBlendState(false, BLEND_OPERATION_ADD, BLEND_FACTOR_ONE, BLEND_FACTOR_ZERO, BLEND_OPERATION_ADD, BLEND_FACTOR_ONE, BLEND_FACTOR_ZERO);
			pipeline.depthStencilState = DepthStencilState(true, true, COMPARISON_LESS, false, StencilFace(), StencilFace());
			pipeline.inputLayout.vertexSize = sizeof(Vertex);
			pipeline.inputLayout.numVertexAttributes = 2;
			pipeline.inputLayout.vertexAttributes[0].location = 0;
			pipeline.inputLayout.vertexAttributes[0].name = "POSITION";
			pipeline.inputLayout.vertexAttributes[0].offset = offsetof(Vertex, Vertex::pos);
			pipeline.inputLayout.vertexAttributes[0].type = VERTEX_ATTRIBUTE_FLOAT3;
			pipeline.inputLayout.vertexAttributes[1].location = 1;
			pipeline.inputLayout.vertexAttributes[1].name = "TEXCOORD";
			pipeline.inputLayout.vertexAttributes[1].offset = offsetof(Vertex, Vertex::texCoord);
			pipeline.inputLayout.vertexAttributes[1].type = VERTEX_ATTRIBUTE_FLOAT2;
			return pipeline;
		}
	};

} }
