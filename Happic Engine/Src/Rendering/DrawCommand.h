#pragma once

#include "../DataStructures/Types.h"
#include "IBuffer.h"
#include "Material.h"


namespace Happic { namespace Rendering {

	enum DrawCommandType
	{
		DRAW_COMMAND_DRAW,
		DRAW_COMMAND_DRAW_INSTANCED,
		DRAW_COMMAND_DRAW_INDEXED,
		DRAW_COMMAND_DRAW_INDEXED_INSTANCED
	};

	struct SubDraw
	{
		SubDraw(uint32 startIndex, uint32 endIndex, uint32 textureGroupIndex) :
			startIndex(startIndex),
			endIndex(endIndex),
			textureGroupIndex(textureGroupIndex)
		{}

		uint32 startIndex;
		uint32 endIndex;
		uint32 textureGroupIndex;
	};

	struct DrawCommand
	{
		DrawCommandType					type;
		
		IBuffer*						pVertexBuffer;
		IBuffer*						pIndexBuffer;

		inline void AddTextureGroup(const TextureGroup& textureGroup) { textureGroups.push_back(textureGroup); }
		inline void AddSubDraw(uint32 startIndex, uint32 endIndex, uint32 textureGroupIndex) { subDraws.push_back(SubDraw(startIndex, endIndex, textureGroupIndex)); }

		std::vector<SubDraw>			subDraws;
		std::vector<TextureGroup>		textureGroups;
	};

} }
