#pragma once

#include "../../DataStructures/String.h"

namespace Happic { namespace Rendering {

	enum ShaderType
	{
		SHADER_TYPE_VERTEX,
		SHADER_TYPE_FRAGMENT,

		NUM_SHADER_TYPES,
		SHADER_NONE
	};

	struct ShaderInfo
	{
		ShaderInfo(const String& vertexShader, const String& fragmentShader) 
		{
			shaderPaths[SHADER_TYPE_VERTEX] = vertexShader;
			shaderPaths[SHADER_TYPE_FRAGMENT] = fragmentShader;
		}

		ShaderInfo() {}

		String shaderPaths[NUM_SHADER_TYPES];
	};

} }
