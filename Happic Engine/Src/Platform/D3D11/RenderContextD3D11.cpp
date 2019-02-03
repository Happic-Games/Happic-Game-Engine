#include "RenderContextD3D11.h"
#include "BufferD3D11.h"
#include "Texture2DD3D11.h"
#include "TextureSamplerD3D11.h"

namespace Happic { namespace Rendering {

	RenderContextD3D11::RenderContextD3D11()
	{
		m_clearColor[0] = 0.0f;
		m_clearColor[1] = 0.0f;
		m_clearColor[2] = 0.0f;
		m_clearColor[3] = 1.0f;
	}

	RenderContextD3D11::~RenderContextD3D11()
	{
		for (const auto& pipeline : m_loadedGraphicsPipelines)
		{
			pipeline.second.pBlendState->Release();
			pipeline.second.pDepthStencilState->Release();
			pipeline.second.pDepthStencilView->Release();
			pipeline.second.pInputLayout->Release();
			pipeline.second.pRasterizerState->Release();
		}

		m_pDevice->Release();
		m_pDeviceContext->Release();
		m_pRenderTargetView->Release();
		m_pSwapChain->Release();

		for (const auto& shader : m_loadedShaders)
		{
			shader.second.pPixelCode->Release();
			shader.second.pPixelShader->Release();
			shader.second.pVertexCode->Release();
			shader.second.pVertexShader->Release();
		}
	}

	void RenderContextD3D11::ChangeGraphicsPipeline(const GraphicsPipeline & pipeline)
	{
		if (pipeline.id == m_graphicsPipelineSettings.id)
			return;

		const auto&& pipelineIndex = m_loadedGraphicsPipelines.find(pipeline.id);
		if (pipelineIndex != m_loadedGraphicsPipelines.end())
		{
			m_graphicsPipeline = pipelineIndex->second;
			m_pActiveShader = &m_loadedShaders[pipeline.shaderInfo.shaderPaths[SHADER_TYPE_VERTEX]];
			m_pDeviceContext->VSSetShader(m_pActiveShader->pVertexShader, NULL, 0);
			m_pDeviceContext->PSSetShader(m_pActiveShader->pPixelShader, NULL, 0);
			m_pDeviceContext->RSSetState(m_graphicsPipeline.pRasterizerState);
			m_pDeviceContext->IASetInputLayout(m_graphicsPipeline.pInputLayout);
			m_pDeviceContext->RSSetViewports(1, &m_graphicsPipeline.viewport);
			m_pDeviceContext->RSSetScissorRects(1, &m_graphicsPipeline.scissor);
			m_pDeviceContext->OMSetBlendState(m_graphicsPipeline.pBlendState, NULL, 0xffffffff);
			m_pDeviceContext->OMSetDepthStencilState(m_graphicsPipeline.pDepthStencilState, 0xFF);

			return;
		}

		m_graphicsPipelineSettings = pipeline;

		const auto&& index = m_loadedShaders.find(pipeline.shaderInfo.shaderPaths[SHADER_TYPE_VERTEX]);
		if (index == m_loadedShaders.end())
			m_pActiveShader = LoadShader(pipeline.shaderInfo);
		else
			m_pActiveShader = &index->second;

		m_graphicsPipelineSettings = pipeline;


		m_pDeviceContext->VSSetShader(m_pActiveShader->pVertexShader, NULL, 0);
		m_pDeviceContext->PSSetShader(m_pActiveShader->pPixelShader, NULL, 0);

		D3D11_RASTERIZER_DESC rasterizer_desc{};
		rasterizer_desc.ScissorEnable = true;
		rasterizer_desc.CullMode = GetCullMode(pipeline.rasterizerState.cullMode);
		rasterizer_desc.FrontCounterClockwise = pipeline.rasterizerState.frontFace == FRONT_FACE_CCW ? true : false;
		rasterizer_desc.FillMode = GetFillMode(pipeline.rasterizerState.polygonMode);

		HRESULT err = m_pDevice->CreateRasterizerState(&rasterizer_desc, &m_graphicsPipeline.pRasterizerState);
		if (err != S_OK)
		{
			std::cerr << "Error creating D3D11 rasterizer state" << std::endl;
			return;
		}

		m_pDeviceContext->RSSetState(m_graphicsPipeline.pRasterizerState);

		std::vector<D3D11_INPUT_ELEMENT_DESC> input_element_descs;
		input_element_descs.resize(pipeline.inputLayout.numVertexAttributes);
		uint32 offset = 0;
		for (uint32 i = 0; i < pipeline.inputLayout.numVertexAttributes; i++)
		{
			D3D11_INPUT_ELEMENT_DESC input_element_desc{};
			input_element_desc.SemanticName = pipeline.inputLayout.vertexAttributes[i].name.C_Str();
			input_element_desc.SemanticIndex = 0;
			input_element_desc.InstanceDataStepRate = 0;
			input_element_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			input_element_desc.InputSlot = 0;
			input_element_desc.AlignedByteOffset = offset;

			switch (pipeline.inputLayout.vertexAttributes[i].type)
			{
			case VERTEX_ATTRIBUTE_FLOAT:  input_element_desc.Format = DXGI_FORMAT_R32_FLOAT; offset += sizeof(float) * 1; break;
			case VERTEX_ATTRIBUTE_FLOAT2: input_element_desc.Format = DXGI_FORMAT_R32G32_FLOAT; offset += sizeof(float) * 2; break;
			case VERTEX_ATTRIBUTE_FLOAT3: input_element_desc.Format = DXGI_FORMAT_R32G32B32_FLOAT; offset += sizeof(float) * 3; break;
			case VERTEX_ATTRIBUTE_FLOAT4: input_element_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; offset += sizeof(float) * 4; break;
			}

			input_element_descs[pipeline.inputLayout.vertexAttributes[i].location] = input_element_desc;
		}

		err = m_pDevice->CreateInputLayout(&input_element_descs[0], input_element_descs.size(), m_pActiveShader->pVertexCode->GetBufferPointer(),
			m_pActiveShader->pVertexCode->GetBufferSize(), &m_graphicsPipeline.pInputLayout);

		if (err != S_OK)
		{
			std::cerr << "Error creating D3D11 input layout" << std::endl;
			return;
		}

		m_pDeviceContext->IASetInputLayout(m_graphicsPipeline.pInputLayout);

		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = pipeline.viewport.x;
		viewport.TopLeftY = pipeline.viewport.y;
		viewport.Width = pipeline.viewport.width;
		viewport.Height = pipeline.viewport.height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D11_RECT rect;
		rect.left = pipeline.scissor.x;
		rect.right = pipeline.scissor.width + pipeline.scissor.x;
		rect.top = pipeline.scissor.y;
		rect.bottom = pipeline.scissor.height + pipeline.scissor.y;

		m_graphicsPipeline.viewport = viewport;
		m_graphicsPipeline.scissor = rect;

		m_pDeviceContext->RSSetViewports(1, &viewport);
		m_pDeviceContext->RSSetScissorRects(1, &rect);

		D3D11_RENDER_TARGET_BLEND_DESC blend_desc;
		blend_desc.BlendEnable = pipeline.colorBlendState.enabled;
		blend_desc.BlendOp = GetBlendOperation(pipeline.colorBlendState.operation);
		blend_desc.BlendOpAlpha = GetBlendOperation(pipeline.colorBlendState.alphaBlendOperation);
		blend_desc.DestBlend = GetBlendFactor(pipeline.colorBlendState.destFactor);
		blend_desc.DestBlendAlpha = GetBlendFactor(pipeline.colorBlendState.alphaDestFactor);
		blend_desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blend_desc.SrcBlend = GetBlendFactor(pipeline.colorBlendState.srcFactor);
		blend_desc.SrcBlendAlpha = GetBlendFactor(pipeline.colorBlendState.alphaSrcFactor);

		D3D11_BLEND_DESC blend_desc_;
		blend_desc_.AlphaToCoverageEnable = false;
		blend_desc_.IndependentBlendEnable = false;
		blend_desc_.RenderTarget[0] = blend_desc;

		err = m_pDevice->CreateBlendState(&blend_desc_, &m_graphicsPipeline.pBlendState);
		if (err != S_OK)
		{
			std::cerr << "Error creating D3D11 blend state" << std::endl;
			return;
		}

		m_pDeviceContext->OMSetBlendState(m_graphicsPipeline.pBlendState, NULL, 0xffffffff);

		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;

		depth_stencil_desc.DepthEnable = pipeline.depthStencilState.depthTestEnabled;
		depth_stencil_desc.DepthWriteMask = pipeline.depthStencilState.depthWriteEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;

		switch (pipeline.depthStencilState.depthComparison)
		{
		case COMPARISON_ALWAYS: depth_stencil_desc.DepthFunc = D3D11_COMPARISON_ALWAYS; break;
		case COMPARISON_NEVER: depth_stencil_desc.DepthFunc = D3D11_COMPARISON_NEVER; break;
		case COMPARISON_LESS: depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS; break;
		case COMPARISON_GREATER: depth_stencil_desc.DepthFunc = D3D11_COMPARISON_GREATER; break;
		case COMPARISON_LESS_EQUAL: depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; break;
		case COMPARISON_GREATER_EQUAL: depth_stencil_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL; break;
		case COMPARISON_EQUAL: depth_stencil_desc.DepthFunc = D3D11_COMPARISON_EQUAL; break;
		case COMPARISON_NOT_EQUAL: depth_stencil_desc.DepthFunc = D3D11_COMPARISON_NOT_EQUAL; break;
		}

		depth_stencil_desc.StencilEnable = pipeline.depthStencilState.stencilTestEnabled;
		depth_stencil_desc.StencilReadMask = 0xFF;
		depth_stencil_desc.StencilWriteMask = 0xFF;

		depth_stencil_desc.BackFace = GetStencilFace(pipeline.depthStencilState.back);
		depth_stencil_desc.FrontFace = GetStencilFace(pipeline.depthStencilState.front);

		m_pDevice->CreateDepthStencilState(&depth_stencil_desc, &m_graphicsPipeline.pDepthStencilState);
		m_pDeviceContext->OMSetDepthStencilState(m_graphicsPipeline.pDepthStencilState, 0xFF);
		InitDepthStencilView();
	}

	void RenderContextD3D11::BeginFrame() const
	{
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_graphicsPipeline.pDepthStencilView);
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, m_clearColor);
		m_pDeviceContext->ClearDepthStencilView(m_graphicsPipeline.pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0xFF);
	}

	void RenderContextD3D11::UpdateTexturesAndSamplers(const TextureGroup& textureGroup) const
	{
		for (uint32 i = 0; i < m_pActiveShader->textures.size(); i++)
		{
			ITexture2D* pTexture = textureGroup.GetTexture(m_pActiveShader->textures[i].name);
			ID3D11ShaderResourceView* pView = ((Texture2DD3D11*)pTexture)->GetShaderResourceView();
			m_pDeviceContext->PSSetShaderResources(m_pActiveShader->textures[i].binding, 1, &pView);
		}
		for (uint32 i = 0; i < m_pActiveShader->samplers.size(); i++)
		{
			ITextureSampler* pSampler = textureGroup.GetSampler(m_pActiveShader->samplers[i].name);
			ID3D11SamplerState* pSamplerState = ((TextureSamplerD3D11*)pSampler)->GetSamplerState();
			m_pDeviceContext->PSSetSamplers(m_pActiveShader->samplers[i].binding, 1, &pSamplerState);
		}
	}

	void RenderContextD3D11::UpdatePerDrawInstanceBuffer(ShaderType type, const void * pData)
	{
		BufferD3D11* pBuffer = (BufferD3D11*)m_pActiveShader->uniformBuffers[type].perDrawInstanceBuffer.pConstantBuffer;
		void* pBufferData = pBuffer->Map();
		memcpy(pBufferData, pData, pBuffer->GetSize());
		pBuffer->Unmap();
		ID3D11Buffer* pBufferD3D11 = pBuffer->GetBuffer();
		if (type == SHADER_TYPE_VERTEX)
			m_pDeviceContext->VSSetConstantBuffers(m_pActiveShader->uniformBuffers[type].perDrawInstanceBuffer.binding, 1, &pBufferD3D11);
		else if (type == SHADER_TYPE_FRAGMENT)
			m_pDeviceContext->PSSetConstantBuffers(m_pActiveShader->uniformBuffers[type].perDrawInstanceBuffer.binding, 1, &pBufferD3D11);
	}

	void RenderContextD3D11::SubmitDrawCommand(const DrawCommand & drawCommand) const
	{
		uint32 stride = m_graphicsPipelineSettings.inputLayout.vertexSize;
		uint32 offset = 0;
		ID3D11Buffer* pVB = ((BufferD3D11*)drawCommand.pVertexBuffer)->GetBuffer();
		m_pDeviceContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);

		if (drawCommand.type == DRAW_COMMAND_DRAW_INDEXED)
		{
			m_pDeviceContext->IASetIndexBuffer(((BufferD3D11*)drawCommand.pIndexBuffer)->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
		}

		for (uint32 i = 0; i < drawCommand.subDraws.size(); i++)
		{
			const SubDraw& subDraw = drawCommand.subDraws[i];
			UpdateTexturesAndSamplers(drawCommand.textureGroups[subDraw.textureGroupIndex]);

			if (drawCommand.type == DRAW_COMMAND_DRAW)
			{
				m_pDeviceContext->Draw(subDraw.endIndex - subDraw.startIndex + 1, subDraw.startIndex);
			}
			else if (drawCommand.type == DRAW_COMMAND_DRAW_INDEXED)
			{
				m_pDeviceContext->IASetIndexBuffer(((BufferD3D11*)drawCommand.pIndexBuffer)->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
				m_pDeviceContext->DrawIndexed(subDraw.endIndex - subDraw.startIndex + 1, subDraw.startIndex, 0);
			}
		}
	}

	void RenderContextD3D11::Swap()
	{
		m_pSwapChain->Present(1, 0);
	}

	void RenderContextD3D11::Init(IDisplay* pDisplay)
	{
		InitDeviceAndSwapChain((Win32Display*)pDisplay);
		InitRenderTargetView();
	}

	ID3D11Device * RenderContextD3D11::GetDevice()
	{
		return m_pDevice;
	}

	ID3D11DeviceContext * RenderContextD3D11::GetDeviceContext()
	{
		return m_pDeviceContext;
	}

	void RenderContextD3D11::InitDeviceAndSwapChain(Win32Display* pDisplay)
	{
		DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
		swap_chain_desc.BufferCount = 1;
		swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.OutputWindow = pDisplay->GetWindowHandle();
		swap_chain_desc.SampleDesc.Count = 1;
		swap_chain_desc.Windowed = true;

		HRESULT err = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
			&swap_chain_desc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext);

		if (err != S_OK)
		{
			std::cerr << "Error creating d3d11 device and swapchain" << std::endl;
			return;
		}
	}

	void RenderContextD3D11::InitRenderTargetView()
	{
		ID3D11Texture2D* pBackBuffer;
		m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);
		pBackBuffer->Release();
	}

	void RenderContextD3D11::InitDepthStencilView()
	{
		DXGI_FORMAT depth_stencil_format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		D3D11_TEXTURE2D_DESC depth_stencil_texture_desc{};

		depth_stencil_texture_desc.Width = m_graphicsPipelineSettings.viewport.width;
		depth_stencil_texture_desc.Height = m_graphicsPipelineSettings.viewport.height;
		depth_stencil_texture_desc.MipLevels = 1;
		depth_stencil_texture_desc.ArraySize = 1;
		depth_stencil_texture_desc.Format = depth_stencil_format;
		depth_stencil_texture_desc.SampleDesc.Count = 1;
		depth_stencil_texture_desc.SampleDesc.Quality = 0;
		depth_stencil_texture_desc.Usage = D3D11_USAGE_DEFAULT;
		depth_stencil_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depth_stencil_texture_desc.CPUAccessFlags = 0;
		depth_stencil_texture_desc.MiscFlags = 0;

		ID3D11Texture2D* pDepthStencilTexture;
		HRESULT err = m_pDevice->CreateTexture2D(&depth_stencil_texture_desc, NULL, &pDepthStencilTexture);
		if (err != S_OK)
		{
			std::cerr << "Error creating d3d11 depth stencil texture" << std::endl;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;

		depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_stencil_view_desc.Texture2D.MipSlice = 0;
		depth_stencil_view_desc.Flags = 0;

		err = m_pDevice->CreateDepthStencilView(pDepthStencilTexture, &depth_stencil_view_desc, &m_graphicsPipeline.pDepthStencilView);

		if (err != S_OK)
		{
			std::cerr << "Error creating d3d11 depth stencil view" << std::endl;
		}

		pDepthStencilTexture->Release();
	}

	ShaderInfoD3D11* RenderContextD3D11::LoadShader(const ShaderInfo & shaderInfo)
	{
		ShaderInfoD3D11 loadedShader;
		loadedShader.name = shaderInfo.shaderPaths[SHADER_TYPE_VERTEX];

		String rootPath = "Res/Shaders/HLSL/";
		String vertexPath = rootPath + shaderInfo.shaderPaths[SHADER_TYPE_VERTEX];
		String pixelPath = rootPath + shaderInfo.shaderPaths[SHADER_TYPE_FRAGMENT];

		String vertexShader = ReadFile(vertexPath.C_Str());
		String pixelShader = ReadFile(pixelPath.C_Str());

		ID3DBlob* pVertexCode = CompileShader(vertexPath, vertexShader, "main", "vs_5_0");
		ID3DBlob* pPixelCode = CompileShader(pixelPath, pixelShader, "main", "ps_5_0");

		ID3D11ShaderReflection* pShaderReflection;
		D3DReflect(pVertexCode->GetBufferPointer(), pVertexCode->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pShaderReflection);
		ParseShader(pShaderReflection, &loadedShader, SHADER_TYPE_VERTEX);
		pShaderReflection->Release();

		D3DReflect(pPixelCode->GetBufferPointer(), pPixelCode->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pShaderReflection);
		ParseShader(pShaderReflection, &loadedShader, SHADER_TYPE_FRAGMENT);
		pShaderReflection->Release();

		loadedShader.pVertexCode = pVertexCode;
		loadedShader.pPixelCode = pPixelCode;

		HRESULT err = m_pDevice->CreateVertexShader(pVertexCode->GetBufferPointer(), pVertexCode->GetBufferSize(), NULL, &loadedShader.pVertexShader);
		if (err != S_OK)
		{
			std::cerr << "Error creating D3D11 vertex shader" << std::endl;
			return NULL;
		}

		err = m_pDevice->CreatePixelShader(pPixelCode->GetBufferPointer(), pPixelCode->GetBufferSize(), NULL, &loadedShader.pPixelShader);
		if (err != S_OK)
		{
			std::cerr << "Error creating D3D11 pixel shader" << std::endl;
			return NULL;
		}

		String key = shaderInfo.shaderPaths[SHADER_TYPE_VERTEX];
		m_loadedShaders[key] = loadedShader;
		return &m_loadedShaders[key];
	}

	void RenderContextD3D11::ParseShader(ID3D11ShaderReflection * pShaderReflection, ShaderInfoD3D11 * pShaderInfo, const ShaderType shaderType)
	{
		D3D11_SHADER_DESC shader_desc {};
		pShaderReflection->GetDesc(&shader_desc);

		for (uint32 i = 0; i < shader_desc.BoundResources; i++)
		{
			D3D11_SHADER_INPUT_BIND_DESC shader_input_bind_desc {};
			pShaderReflection->GetResourceBindingDesc(i, &shader_input_bind_desc);

			if (shader_input_bind_desc.Type == D3D_SIT_CBUFFER)
			{
				ID3D11ShaderReflectionConstantBuffer* pShaderRelfectionConstantBuffer = pShaderReflection->GetConstantBufferByIndex(i);
				D3D11_SHADER_BUFFER_DESC shader_buffer_desc {};
				pShaderRelfectionConstantBuffer->GetDesc(&shader_buffer_desc);
			
				if (strcmp(shader_input_bind_desc.Name, "PerWindowResize") == 0)
				{
					pShaderInfo->uniformBuffers[shaderType].perWindowResizeBuffer.pConstantBuffer = (BufferD3D11*)
						IBuffer::CreateBuffer(BUFFER_TYPE_UNIFORM, BUFFER_USAGE_DYNAMIC, NULL, shader_buffer_desc.Size);
					pShaderInfo->uniformBuffers[shaderType].perWindowResizeBuffer.binding = shader_input_bind_desc.BindPoint;
				}
				else if (strcmp(shader_input_bind_desc.Name, "PerFrame") == 0)
				{
					pShaderInfo->uniformBuffers[shaderType].perFrameBuffer.pConstantBuffer = (BufferD3D11*)
						IBuffer::CreateBuffer(BUFFER_TYPE_UNIFORM, BUFFER_USAGE_DYNAMIC, NULL, shader_buffer_desc.Size);
					pShaderInfo->uniformBuffers[shaderType].perFrameBuffer.binding = shader_input_bind_desc.BindPoint;
				}
				else if (strcmp(shader_input_bind_desc.Name, "PerDrawInstance") == 0)
				{
					pShaderInfo->uniformBuffers[shaderType].perDrawInstanceBuffer.pConstantBuffer = (BufferD3D11*)
						IBuffer::CreateBuffer(BUFFER_TYPE_UNIFORM, BUFFER_USAGE_DYNAMIC, NULL, shader_buffer_desc.Size);
					pShaderInfo->uniformBuffers[shaderType].perDrawInstanceBuffer.binding = shader_input_bind_desc.BindPoint;
				}
			}
			else if (shader_input_bind_desc.Type == D3D_SIT_TEXTURE)
			{
				ResourceInfoD3D11 texture;
				texture.name = shader_input_bind_desc.Name;
				texture.binding = shader_input_bind_desc.BindPoint;
				pShaderInfo->textures.push_back(texture);
			}
			else if (shader_input_bind_desc.Type == D3D_SIT_SAMPLER)
			{
				ResourceInfoD3D11 sampler;
				sampler.name = shader_input_bind_desc.Name;
				sampler.binding = shader_input_bind_desc.BindPoint;
				pShaderInfo->samplers.push_back(sampler);
			}
		}
	}

	D3D11_CULL_MODE RenderContextD3D11::GetCullMode(CullMode cullMode)
	{
		switch (cullMode)
		{
		case CULL_MODE_FRONT: return D3D11_CULL_FRONT;
		case CULL_MODE_BACK: return D3D11_CULL_BACK;
		case CULL_MODE_FRONT_BACK: return D3D11_CULL_BACK;
		case CULL_MODE_NONE: return D3D11_CULL_NONE;
		}
	}

	D3D11_FILL_MODE RenderContextD3D11::GetFillMode(PolygonMode fillMode)
	{
		switch (fillMode)
		{
		case POLYGON_MODE_FILL: return D3D11_FILL_SOLID;
		case POLYGON_MODE_LINE: return D3D11_FILL_WIREFRAME;
		case POLYGON_MODE_POINT: return D3D11_FILL_WIREFRAME;
		}
	}

	D3D11_BLEND_OP RenderContextD3D11::GetBlendOperation(BlendOperation operation)
	{
		switch (operation)
		{
		case BLEND_OPERATION_ADD: return D3D11_BLEND_OP_ADD;
		case BLEND_OPERATION_SUB: return D3D11_BLEND_OP_SUBTRACT;
		case BLEND_OPERATION_REV_SUB: return D3D11_BLEND_OP_REV_SUBTRACT;
		case BLEND_OPERATION_MIN: return D3D11_BLEND_OP_MIN;
		case BLEND_OPERATION_MAX: return D3D11_BLEND_OP_MAX;
		}
	}

	D3D11_BLEND RenderContextD3D11::GetBlendFactor(BlendFactor factor)
	{
		switch (factor)
		{
		case BLEND_FACTOR_ZERO: return D3D11_BLEND_ZERO;
		case BLEND_FACTOR_ONE: return D3D11_BLEND_ONE;
		case BLEND_FACTOR_SRC_COLOR: return D3D11_BLEND_SRC_COLOR;
		case BLEND_FACTOR_INV_SOURCE_COLOR: return D3D11_BLEND_INV_SRC_COLOR;
		case BLEND_FACTOR_DEST_COLOR: return D3D11_BLEND_DEST_COLOR;
		case BLEND_FACTOR_INV_DEST_COLOR: return D3D11_BLEND_INV_DEST_COLOR;
		case BLEND_FACTOR_SRC_ALPHA: return D3D11_BLEND_SRC_ALPHA;
		case BLEND_FACTOR_SRC_INV_ALPHA: return D3D11_BLEND_INV_SRC_ALPHA;
		case BLEND_FACTOR_DEST_ALPHA: return D3D11_BLEND_DEST_ALPHA;
		case BLEND_FACTOR_DEST_INV_ALPHA: return D3D11_BLEND_INV_DEST_ALPHA;
		}
	}

	D3D11_DEPTH_STENCILOP_DESC RenderContextD3D11::GetStencilFace(const StencilFace & face)
	{
		D3D11_DEPTH_STENCILOP_DESC stencil_op_desc;
		stencil_op_desc.StencilDepthFailOp = GetStencilOperation(face.depthStencilFail);
		stencil_op_desc.StencilFailOp = GetStencilOperation(face.stencilFail);
		stencil_op_desc.StencilPassOp = GetStencilOperation(face.depthStencilPass);

		switch (face.comparison)
		{
		case COMPARISON_NEVER: stencil_op_desc.StencilFunc = D3D11_COMPARISON_NEVER; break;
		case COMPARISON_LESS: stencil_op_desc.StencilFunc = D3D11_COMPARISON_LESS; break;
		case COMPARISON_LESS_EQUAL: stencil_op_desc.StencilFunc = D3D11_COMPARISON_LESS_EQUAL; break;
		case COMPARISON_GREATER: stencil_op_desc.StencilFunc = D3D11_COMPARISON_GREATER; break;
		case COMPARISON_GREATER_EQUAL: stencil_op_desc.StencilFunc = D3D11_COMPARISON_GREATER_EQUAL; break;
		case COMPARISON_EQUAL: stencil_op_desc.StencilFunc = D3D11_COMPARISON_EQUAL; break;
		case COMPARISON_NOT_EQUAL: stencil_op_desc.StencilFunc = D3D11_COMPARISON_NOT_EQUAL; break;
		case COMPARISON_ALWAYS: stencil_op_desc.StencilFunc = D3D11_COMPARISON_ALWAYS; break;
		}

		return stencil_op_desc;
	}

	D3D11_STENCIL_OP RenderContextD3D11::GetStencilOperation(StencilOperation operation)
	{
		switch (operation)
		{
		case STENCIL_OPERATION_KEEP: return D3D11_STENCIL_OP_KEEP;
		case STENCIL_OPERATION_ZERO: return D3D11_STENCIL_OP_ZERO;
		case STENCIL_OPERATION_REPLACE: return D3D11_STENCIL_OP_REPLACE;
		case STENCIL_OPERATION_INCR_CLAMP: return D3D11_STENCIL_OP_INCR_SAT;
		case STENCIL_OPERATION_DECR_CLAMP: return D3D11_STENCIL_OP_DECR_SAT;
		case STENCIL_OPERATION_INVERT: return D3D11_STENCIL_OP_INVERT;
		case STENCIL_OPERATION_INCR: return D3D11_STENCIL_OP_INCR;
		case STENCIL_OPERATION_DECR: return D3D11_STENCIL_OP_DECR;
		}
	}

	ID3DBlob * RenderContextD3D11::CompileShader(const String & sourceName, const String & shaderSrc, const String & entryPoint, const String & target)
	{
		ID3DBlob* pCode;
		ID3DBlob* pErrMsgs;

		D3DCompile(shaderSrc.C_Str(), shaderSrc.Length(), sourceName.C_Str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.C_Str(),
			target.C_Str(), D3DCOMPILE_DEBUG, 0, &pCode, &pErrMsgs);

		if (pErrMsgs)
		{
			std::cerr << "Error creating D3D11 shader :" << std::endl << ((char*)pErrMsgs->GetBufferPointer()) << std::endl;
			pErrMsgs->Release();
			return NULL;
		}

		return pCode;
	}

	String RenderContextD3D11::ReadFile(cstring path)
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

		return loadedFile;
	}

} }
