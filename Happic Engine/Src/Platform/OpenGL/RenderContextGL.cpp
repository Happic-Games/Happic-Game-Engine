#include "RenderContextGL.h"
#include "BufferGL.h"
#include <GL\glew.h>
#include <GL\wglew.h>
#include "../Windows/Win32Display.h"
#include <string>
#include "Texture2DGL.h"
#include "TextureSamplerGL.h"

namespace Happic { namespace Rendering {

	RenderContextGL::RenderContextGL()
	{
		
	}

	RenderContextGL::~RenderContextGL()
	{
	}

	void RenderContextGL::Init(const RenderContextInitInfo & initInfo)
	{
		m_graphicsPipelineSettings = initInfo.pipeline;
		m_hdc = GetDC(((Win32Display*)initInfo.pDisplay)->GetWindowHandle());
		HGLRC glrc = wglCreateContext(m_hdc);
		
		if (glrc)
		{
			if (!wglMakeCurrent(m_hdc, glrc))
			{
				std::cerr << "Could not create opengl context" << std::endl;
				return;
			}
		}
		else
		{
			std::cerr << "Could not create opengl context" << std::endl;
			return;
		}

		if (glewInit() != GLEW_OK)
		{
			std::cerr << "Could not initialize glew" << std::endl;
			return;
		}

		wglSwapIntervalEXT(1);

		uint32 shaderID = LoadShaderProgram(initInfo.pipeline.shaderInfo.shaderPaths[SHADER_TYPE_VERTEX].C_Str(),
			initInfo.pipeline.shaderInfo.shaderPaths[SHADER_TYPE_FRAGMENT].C_Str());

		if (initInfo.pipeline.depthStencilState.depthTestEnabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		if (initInfo.pipeline.depthStencilState.depthWriteEnabled)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(FALSE);

		if (initInfo.pipeline.depthStencilState.stencilTestEnabled)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);


		glDepthFunc(GetDepthFunc(initInfo.pipeline.depthStencilState.depthComparison));
		glStencilOpSeparate(GL_FRONT, GetStencilOp(initInfo.pipeline.depthStencilState.front.stencilFail),
			GetStencilOp(initInfo.pipeline.depthStencilState.front.depthStencilFail), GetStencilOp(initInfo.pipeline.depthStencilState.front.depthStencilPass));
		
		glStencilOpSeparate(GL_BACK, GetStencilOp(initInfo.pipeline.depthStencilState.back.stencilFail),
			GetStencilOp(initInfo.pipeline.depthStencilState.back.depthStencilFail), GetStencilOp(initInfo.pipeline.depthStencilState.back.depthStencilPass));

		glStencilFuncSeparate(GL_FRONT, GetDepthFunc(initInfo.pipeline.depthStencilState.front.comparison), 0xFF, 0xFF);

		glFrontFace(initInfo.pipeline.rasterizerState.frontFace == FRONT_FACE_CCW ? GL_CCW : GL_CW);

		m_pActiveShader = &m_loadedShaders[shaderID];

		glUseProgram(m_pActiveShader->program);
	}

	void RenderContextGL::BeginFrame() const
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void RenderContextGL::SetTextureUniforms(const TextureGroup& textureGroup) const
	{
		for (uint32 i = 0; i < m_pActiveShader->samplers.size(); i++)
		{
			TextureSamplerPair pair = textureGroup.GetTextureAndSampler(m_pActiveShader->samplers[i].name);
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, ((Texture2DGL*)pair.pTexture)->GetTexture());
			((TextureSamplerGL*)pair.pSampler)->BindSampler();
			glUniform1i(m_pActiveShader->samplers[i].binding, i);
		}
	}

	void RenderContextGL::UpdatePerDrawInstanceBuffer(ShaderType type, const void * pData)
	{
		void* pBufferData = m_pActiveShader->uniformBuffers[type].perDrawInstanceBuffer->Map();

		memcpy(pBufferData, pData, m_pActiveShader->uniformBuffers[type].perDrawInstanceBuffer->GetSize());
		m_pActiveShader->uniformBuffers[type].perDrawInstanceBuffer->Unmap();

		glBindBufferBase(GL_UNIFORM_BUFFER, m_pActiveShader->uniformBuffers[type].perDrawInstanceBufferBinding,
			m_pActiveShader->uniformBuffers[type].perDrawInstanceBuffer->GetBuffer());
	}

	void RenderContextGL::SubmitDrawCommand(const DrawCommand & drawCommand) const
	{
		BufferGL* pVB = (BufferGL*)drawCommand.pVertexBuffer;
		glBindVertexArray(pVB->GetVao());

		if (drawCommand.type == DRAW_COMMAND_DRAW_INDEXED)
		{
			BufferGL* pIB = (BufferGL*)drawCommand.pIndexBuffer;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIB->GetBuffer());
		}

		for (uint32 i = 0; i < drawCommand.subDraws.size(); i++)
		{
			const SubDraw& subDraw = drawCommand.subDraws[i];
			SetTextureUniforms(drawCommand.textureGroups[subDraw.textureGroupIndex]);

			if (drawCommand.type == DRAW_COMMAND_DRAW)
			{
				glDrawArrays(GL_TRIANGLES, subDraw.startIndex, subDraw.endIndex - subDraw.startIndex + 1);
			}
			else if (drawCommand.type == DRAW_COMMAND_DRAW_INDEXED)
			{
				glDrawElements(GL_TRIANGLES, subDraw.endIndex - subDraw.startIndex + 1, GL_UNSIGNED_INT, (const GLvoid*)(subDraw.startIndex * sizeof(uint32)));
			}
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void RenderContextGL::Swap()
	{
		SwapBuffers(m_hdc);
	}

	const VertexInputLayout & RenderContextGL::GetInputLayout() const
	{
		return m_graphicsPipelineSettings.inputLayout;
	}

	void RenderContextGL::ParseShader(const String & glsl, ShaderInfoGL * pShaderInfo, ShaderType type)
	{
		int32 layoutIndex = glsl.FindFirstOf("layout");
		uint32 bindingIndexValue = 0;
		while (layoutIndex != -1)
		{
			int32 locationIndex = glsl.FindFirstOf("location", layoutIndex);
			int32 std140Index = glsl.FindFirstOf("std140", layoutIndex);

			if (std140Index == -1)
				break;

			if (locationIndex < std140Index)
			{
				layoutIndex = glsl.FindFirstOf("layout", locationIndex);
				continue;
			}

			//find the ubo name
			int32 uniformIndex = glsl.FindFirstOf("uniform", std140Index);
			int32 leftBraceIndex = glsl.FindFirstOf("{", uniformIndex);
			String uboName = glsl.SubString(uniformIndex + 7, leftBraceIndex);
			uboName = uboName.Trim();

			int32 blockIndex = glGetUniformBlockIndex(pShaderInfo->program, uboName.C_Str());
			glUniformBlockBinding(pShaderInfo->program, blockIndex, bindingIndexValue);

			//calculate buffer size
			uint32 bufferSize = 0;
			int32 rightBraceIndex = glsl.FindFirstOf("}", leftBraceIndex);
			int32 startMemberIndex = leftBraceIndex;
			int32 endMemberIndex = glsl.FindFirstOf(";", startMemberIndex);
			while (endMemberIndex < rightBraceIndex)
			{
				int32 memberTypeIndex = glsl.FindFirstCharacter(startMemberIndex + 1);
				int32 spaceIndex = glsl.FindFirstSpace(memberTypeIndex);
				String memberType = glsl.SubString(memberTypeIndex, spaceIndex);

				if (memberType == "float")
					bufferSize += sizeof(float);
				else if (memberType == "vec2")
					bufferSize += sizeof(float) * 2;
				else if (memberType == "vec3")
					bufferSize += sizeof(float) * 3;
				else if (memberType == "vec4")
					bufferSize += sizeof(float) * 4;
				else if (memberType == "mat2")
					bufferSize += sizeof(float) * 4;
				else if (memberType == "mat3")
					bufferSize += sizeof(float) * 9;
				else if (memberType == "mat4")
					bufferSize += sizeof(float) * 16;

				startMemberIndex = endMemberIndex;
				endMemberIndex = glsl.FindFirstOf(";", startMemberIndex + 1);
			}


			if (uboName == "PerWindowResize")
			{
				pShaderInfo->uniformBuffers[type].perWindowResizeBuffer = (BufferGL*)
					IBuffer::CreateBuffer(BUFFER_TYPE_UNIFORM, BUFFER_USAGE_DYNAMIC, NULL, bufferSize);
				pShaderInfo->uniformBuffers[type].perWindowResizeBufferBinding = bindingIndexValue;
			}
			else if (uboName == "PerFrame")
			{
				pShaderInfo->uniformBuffers[type].perFrameBuffer = (BufferGL*)
					IBuffer::CreateBuffer(BUFFER_TYPE_UNIFORM, BUFFER_USAGE_DYNAMIC, NULL, bufferSize);
				pShaderInfo->uniformBuffers[type].perFrameBufferBinding = bindingIndexValue;
			}
			else if (uboName == "PerDrawInstance")
			{
				pShaderInfo->uniformBuffers[type].perDrawInstanceBuffer = (BufferGL*)
					IBuffer::CreateBuffer(BUFFER_TYPE_UNIFORM, BUFFER_USAGE_DYNAMIC, NULL, bufferSize);
				pShaderInfo->uniformBuffers[type].perDrawInstanceBufferBinding = bindingIndexValue;
			}

			layoutIndex = glsl.FindFirstOf("layout", rightBraceIndex);
		}

		if (type == SHADER_TYPE_FRAGMENT)
		{
			int32 sampler2DIndex = glsl.FindFirstOf("sampler2D");
			while (sampler2DIndex != -1)
			{
				SamplerInfoGL samplerInfo;

				int32 semicolonIndex = glsl.FindFirstOf(";", sampler2DIndex);
				String samplerName = glsl.SubString(sampler2DIndex + 10, semicolonIndex);
				samplerName = samplerName.Trim();
				samplerInfo.name = samplerName;
				int32 location = glGetUniformLocation(pShaderInfo->program, samplerName.C_Str());
				samplerInfo.binding = location;
				pShaderInfo->samplers.push_back(samplerInfo);
				sampler2DIndex = glsl.FindFirstOf("sampler2D", semicolonIndex);
			}
		}
	}

	uint32 RenderContextGL::LoadShaderProgram(cstring vertexName, cstring fragmentName)
	{
		String rootPath = "Res/Shaders/GLSL_330/";

		String vertexPath = rootPath + vertexName;
		String fragmentPath = rootPath + fragmentName;

		String vertexCode = ReadFile(vertexPath.C_Str());
		String fragmentCode = ReadFile(fragmentPath.C_Str());

		ShaderInfoGL loadedShader;
		loadedShader.name = vertexName;
		loadedShader.program = glCreateProgram();
		loadedShader.vertexShader = CreateShader(vertexCode, GL_VERTEX_SHADER, loadedShader.program);
		loadedShader.fragmentShader = CreateShader(fragmentCode, GL_FRAGMENT_SHADER, loadedShader.program);

		glLinkProgram(loadedShader.program);
		CheckForError(loadedShader.program, GL_LINK_STATUS, true, "Error: Failed to link shader program: ");

		glValidateProgram(loadedShader.program);
		CheckForError(loadedShader.program, GL_VALIDATE_STATUS, true, "Error: Failed to validate program: ");

		ParseShader(vertexCode, &loadedShader, SHADER_TYPE_VERTEX);
		ParseShader(fragmentCode, &loadedShader, SHADER_TYPE_FRAGMENT);

		uint32 shaderID = m_loadedShaders.size();
		m_loadedShaders.push_back(loadedShader);
		return shaderID;
	}

	GLuint RenderContextGL::CreateShader(const String & code, GLenum type, GLuint program)
	{
		GLuint shader = glCreateShader(type);
		if (shader == 0)
		{
			std::cerr << "Failder to create opengl shader" << std::endl;
			return 0;
		}

		const GLchar* shaderSourceStrings[1];
		GLint shaderSourceStringLengths[1];

		shaderSourceStringLengths[0] = code.Length();
		shaderSourceStrings[0] = code.C_Str();

		glShaderSource(shader, 1, shaderSourceStrings, shaderSourceStringLengths);
		glCompileShader(shader);
		glAttachShader(program, shader);
		return shader;
	}

	void RenderContextGL::CheckForError(GLuint shader, GLuint flag, bool isProgram, const String& errorMessage)
	{
		GLint success = 0;
		GLchar error[1024] = { 0 };

		if (isProgram)
			glGetProgramiv(shader, flag, &success);
		else
			glGetShaderiv(shader, flag, &success);

		if (success == GL_FALSE)
		{
			if (isProgram)
				glGetProgramInfoLog(shader, sizeof(error), NULL, error);
			else
				glGetShaderInfoLog(shader, sizeof(error), NULL, error);

			std::cerr << errorMessage << ": '" << error << "'" << std::endl;
		}
	}

	String RenderContextGL::ReadFile(cstring path)
	{
		FILE* pFile = fopen(path, "rb");

		if (!pFile)
		{
			std::cerr << "Error finding shader file : " << path << std::endl;
			return "Error";
		}

		fseek(pFile, 0, SEEK_END);
		long size = ftell(pFile);
		fseek(pFile, 0L, SEEK_SET);

		String loadedFile(size);
		fread((void*)loadedFile.C_Str(), size * sizeof(char), 1, pFile);
		fclose(pFile);
		loadedFile = loadedFile.RemoveAllOf("\r");
		return loadedFile;
	}

	GLenum RenderContextGL::GetDepthFunc(Comparison comparison)
	{
		switch (comparison)
		{
		case COMPARISON_LESS: return GL_LESS;
		case COMPARISON_GREATER: return GL_GREATER;
		case COMPARISON_LESS_EQUAL: return GL_LEQUAL;
		case COMPARISON_GREATER_EQUAL: return GL_GEQUAL;
		case COMPARISON_EQUAL: return GL_EQUAL;
		case COMPARISON_NOT_EQUAL: return GL_NOTEQUAL;
		case COMPARISON_NEVER: return GL_NEVER;
		case COMPARISON_ALWAYS: return GL_ALWAYS;
		}
	}

	GLenum RenderContextGL::GetStencilOp(StencilOperation operation)
	{
		switch (operation)
		{
		case STENCIL_OPERATION_KEEP: return GL_KEEP;
		case STENCIL_OPERATION_ZERO: return GL_ZERO;
		case STENCIL_OPERATION_REPLACE: return GL_REPLACE;
		case STENCIL_OPERATION_INCR_CLAMP: return GL_INCR;
		case STENCIL_OPERATION_DECR_CLAMP: return GL_DECR;
		case STENCIL_OPERATION_INVERT: return GL_INVERT;
		case STENCIL_OPERATION_INCR: return GL_INCR_WRAP;
		case STENCIL_OPERATION_DECR: return GL_DECR_WRAP;
		}
	}

} }
