#pragma once

#include "../../Rendering/IRenderContext.h"
#include "../Windows/Win32Display.h"
#include "BufferD3D11.h"

#include <vector>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <map>

namespace Happic { namespace Rendering {

	struct ConstantBufferInfoD3D11
	{
		BufferD3D11*	pConstantBuffer;
		uint32			binding;
	};

	struct ResourceInfoD3D11
	{
		String name;
		uint32 binding;
	};

	struct ConstantBuffersD3D11
	{
		ConstantBufferInfoD3D11 perWindowResizeBuffer;
		ConstantBufferInfoD3D11 perFrameBuffer;
		ConstantBufferInfoD3D11 perDrawInstanceBuffer;
	};

	struct ShaderInfoD3D11
	{
		ID3D11VertexShader*						pVertexShader;
		ID3D11PixelShader*						pPixelShader;
		ID3DBlob*								pVertexCode;
		ID3DBlob*								pPixelCode;
		String									name;
		ConstantBuffersD3D11					uniformBuffers[NUM_SHADER_TYPES];
		std::vector<ResourceInfoD3D11>			textures;
		std::vector<ResourceInfoD3D11>			samplers;
	};

	struct GraphicsPipelineD3D11
	{
		ID3D11DepthStencilView*					pDepthStencilView;
		ID3D11RasterizerState*					pRasterizerState;
		ID3D11InputLayout*						pInputLayout;
		ID3D11BlendState*						pBlendState;
		ID3D11DepthStencilState*				pDepthStencilState;
		D3D11_VIEWPORT							viewport;
		D3D11_RECT								scissor;
	};

	class RenderContextD3D11 : public IRenderContext
	{
	public:
		RenderContextD3D11();
		~RenderContextD3D11();

		void ChangeGraphicsPipeline(const GraphicsPipeline& pipeline);

		void BeginFrame() const override;
		void UpdatePerDrawInstanceBuffer(ShaderType type, const void* pData) override;
		void SubmitDrawCommand(const DrawCommand& drawCommand) const override;
		void Swap() override;

		void Init(IDisplay* pDisplay) override;

		ID3D11Device* GetDevice();
		ID3D11DeviceContext* GetDeviceContext();
	private:
		void InitDeviceAndSwapChain(Win32Display* pDisplay);
		void InitRenderTargetView();
		void InitDepthStencilView();

		void UpdateTexturesAndSamplers(const TextureGroup& textureGroup) const;

		ShaderInfoD3D11* LoadShader(const ShaderInfo& shaderInfo);
		void ParseShader(ID3D11ShaderReflection* pShaderReflection, ShaderInfoD3D11* pShaderInfo, const ShaderType shaderType);
	private:
		static D3D11_CULL_MODE GetCullMode(CullMode cullMode);
		static D3D11_FILL_MODE GetFillMode(PolygonMode fillMode);
		static D3D11_BLEND_OP GetBlendOperation(BlendOperation operation);
		static D3D11_BLEND GetBlendFactor(BlendFactor factor);
		static D3D11_DEPTH_STENCILOP_DESC GetStencilFace(const StencilFace & face);
		static D3D11_STENCIL_OP GetStencilOperation(StencilOperation operation);

		static ID3DBlob* CompileShader(const String& sourceName, const String& shaderSrc, const String& entryPoint, const String& target);
		static String ReadFile(cstring path);
	private:
		IDXGISwapChain*											m_pSwapChain;
		ID3D11Device*											m_pDevice;
		ID3D11DeviceContext*									m_pDeviceContext;

		ID3D11RenderTargetView*									m_pRenderTargetView;
		GraphicsPipelineD3D11									m_graphicsPipeline;

		GraphicsPipeline										m_graphicsPipelineSettings;

		float m_clearColor[4];

		std::map<String, ShaderInfoD3D11>						m_loadedShaders;
		std::map<GraphicsPipelineID, GraphicsPipelineD3D11>		m_loadedGraphicsPipelines;
		ShaderInfoD3D11*										m_pActiveShader;
	};

} }
