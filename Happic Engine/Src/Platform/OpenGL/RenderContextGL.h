#pragma once

#include "../../Rendering/IRenderContext.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GL\glew.h>
#include "BufferGL.h"

namespace Happic { namespace Rendering {

	struct SamplerInfoGL
	{
		String name;
		uint32 binding;
	};

	struct ShaderUniformBuffesGL
	{
		BufferGL*					perWindowResizeBuffer;
		uint32						perWindowResizeBufferBinding;

		BufferGL*					perFrameBuffer;
		uint32						perFrameBufferBinding;

		BufferGL*					perDrawInstanceBuffer;
		uint32						perDrawInstanceBufferBinding;
	};

	struct ShaderInfoGL
	{
		GLuint								program;
		GLuint								vertexShader;
		GLuint								fragmentShader;
		String								name;
		ShaderUniformBuffesGL				uniformBuffers[NUM_SHADER_TYPES];
		std::vector<SamplerInfoGL>			samplers;
	};

	class RenderContextGL : public IRenderContext
	{
	public:
		RenderContextGL();
		~RenderContextGL();

		void Init(const RenderContextInitInfo& initInfo) override;

		void BeginFrame() const override;
		void UpdatePerDrawInstanceBuffer(ShaderType type, const void* pData) override;
		void SubmitDrawCommand(const DrawCommand& drawCommand) const override;
		void Swap() override;

		uint32 LoadShaderProgram(cstring vertexName, cstring fragmentName);

		const VertexInputLayout& GetInputLayout() const;
	private:
		void SetTextureUniforms(const TextureGroup& textureGroup) const;

		void ParseShader(const String& glsl, ShaderInfoGL* pShaderInfo, ShaderType type);

		static GLuint CreateShader(const String& code, GLenum type, GLuint program);
		static void CheckForError(GLuint shader, GLuint flag, bool isProgram, const String& errorMessage);
		static String ReadFile(cstring path);

		static GLenum GetDepthFunc(Comparison comparison);
		static GLenum GetStencilOp(StencilOperation operation);
	private:
		HDC m_hdc;
		GraphicsPipeline m_graphicsPipelineSettings;
		std::vector<ShaderInfoGL> m_loadedShaders;
		ShaderInfoGL* m_pActiveShader;
	};

} }
