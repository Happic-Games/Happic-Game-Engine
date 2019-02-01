#pragma once

#include "../../Rendering/IRenderContext.h"
#include "../../Rendering/IDisplay.h"
#include "../../DataStructures/Types.h"
#include <vulkan\vulkan.h>
#include <vector>
#include <unordered_map>
#include "BufferVK.h"

#define RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT 1
#define RENDER_CONTEXT_VK_MAX_INSTANCES_PER_BUFFER 256

namespace Happic { namespace Rendering {

	struct QueueFamilyIndices
	{
		int32 graphics;
		int32 present;
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR			capabilities;
		std::vector<VkSurfaceFormatKHR>		formats;
		std::vector<VkPresentModeKHR>		presentModes;
	};

	struct ShaderBufferInfoVK
	{
		String				name;
		ShaderType			shaderStage;
		uint32				binding;
		uint32				size;
	};

	struct SamplerInfoVK
	{
		String name;
		uint32 binding;
	};

	struct ShaderInfoVK
	{
		ShaderInfo							shaderInfo;
		VkShaderModule						shaderModules[NUM_SHADER_TYPES];
		VkPipelineShaderStageCreateInfo		shaderStages[NUM_SHADER_TYPES];
		std::vector<ShaderBufferInfoVK>		shaderBuffers;
		std::vector<SamplerInfoVK>			samplers;
	};

	struct DepthStencilBufferVK
	{
		VkImage			depthStencilImage;
		VkImageView		depthStencilImageView;
		VkDeviceMemory	imageMemory;
	};

	struct UniformBufferInfoVK
	{
		IBuffer*	pUniformBuffer;
		uint32		binding;
	};

	struct ShaderUniformBuffersVK
	{
		BufferVK*	perWindowResizeBuffer;
		uint32		perWindowResizeBufferBinding;

		BufferVK*	perFrameBuffer;
		uint32		perFrameBufferBinding;

		BufferVK*	perDrawInstanceBuffer;
		uint32		perDrawInstanceBufferBinding;

		uint32		instanceAlignment;
		void*		pMappedInstanceBufferData;
	};

	struct FrameInFlight
	{
		VkSemaphore										imageAvailableSemaphore;
		VkSemaphore										renderFinishedSemaphore;
		VkFence											fence;
		VkCommandBuffer									renderCommandBuffer;
		ShaderUniformBuffersVK							uniformBuffers[NUM_SHADER_TYPES];
		uint32											dynamicUniformOffsetCount;
		mutable uint32									dynamicUniformOffsets[NUM_SHADER_TYPES];
		VkDescriptorSet									uniformBufferdescriptorSet;
		mutable std::unordered_map<uint32, VkDescriptorSet>		textureDescriptorSets;
	};

	class RenderContextVK : public IRenderContext
	{
	public:
		RenderContextVK();
		~RenderContextVK();

		void Init(const RenderContextInitInfo& initInfo) override;

		void UpdatePerDrawInstanceBuffer(ShaderType type, const void* pData) override;
		void SubmitDrawCommand(const DrawCommand& drawCommand) const override;

		void BeginFrame() const override;
		void Swap() override;
		void DisplayClosed() const override;

		uint32 FindMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties);

		void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer src, VkImage dst, uint32 width, uint32 height);

		bool CreateImage(uint32 width, uint32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* pImage, VkDeviceMemory* pImageMemory);
		bool CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags imageAspectFlags, VkImageView* pImageView);

		const VkDevice& GetDevice() const;
	private:
		void CheckDebugValidationLayers();
		void InitInstance();
		std::vector<cstring> GetRequiredExtensions() const;
		void InitValidationLayers();
		void InitSurface(IDisplay* pDisplay);
		void InitPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice physicalDevice);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
		void InitQueueFamilyIndices();
		void CreateLogicalDevice();
		SwapChainSupportDetails QuerySwapChainSupportDetails(VkPhysicalDevice physicalDevice);
		VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
		VkExtent2D ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32 width, uint32 height);
		void InitSwapChain(uint32 width, uint32 height);
		void InitImageViews();
		void InitRenderPass();
		void InitGraphicsPipeline(const GraphicsPipeline& pipeline);
		void InitFramebuffers();
		void InitCommandPool();
		void InitCommandBuffers();
		void InitSemaphoresAndFences();
		void InitUniformBuffers();
		void InitDescriptorPool();
		void InitDescriptorSets();
		void UpdateDescriptorSet(VkDescriptorSet descriptorSet, const BufferVK* pBuffer, uint32 binding, VkDescriptorType type);
		void InitDepthStencilBuffer();
		void CleanupSwapChain();
		void RecreateSwapChain(uint32 width, uint32 height);

		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

		VkDescriptorSet UpdateSamplerDescriptors(const TextureGroup& textureGroup) const;
		void UpdateUniformBuffer(IBuffer* pBuffer, uint32 offset, const void* pData) const;

		VkFormat FindSupportedFormats(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat FindDepthFormat();
		bool HasStencilComponent(VkFormat format);

		uint32 LoadShader(const ShaderInfo& pShaderInfo, const VkDevice& device);
		VkShaderModule CreateShaderModule(const std::vector<char>& code, const VkDevice& device);
		void ParseShader(const String& glsl, ShaderType type, ShaderInfoVK* pShaderInfo);
	private:
		static VkPrimitiveTopology GetTopology(PrimitiveTopology primitiveTopology);
		static VkPolygonMode GetPolygonMode(PolygonMode polygonMode);
		static VkCullModeFlagBits GetCullMode(CullMode cullMode);
		static VkFrontFace GetFrontFace(FrontFace frontFace);
		static VkCompareOp GetComparison(Comparison comparison);
		static VkStencilOp GetStencilOperation(StencilOperation stencilOperation);
		static VkStencilOpState GetStencilFace(const StencilFace& stencilFace);
		static VkBlendOp GetBlendOperation(BlendOperation blendOperation);
		static VkBlendFactor GetBlendFactor(BlendFactor blendFactor);

		static std::vector<char> ReadFile(cstring path);

		friend bool window_resize_update_swapchain(WindowEvent windowEvent, const WindowEventInfo&  windowEventInfo, void* pUserPtr);
	private:
		VkInstance						m_instance;
		VkDebugUtilsMessengerEXT		m_callback;
		VkPhysicalDevice				m_physicalDevice;
		QueueFamilyIndices				m_queueFamilies;
		VkDevice						m_device;
		VkQueue							m_graphicsQueue;
		VkQueue							m_presentQueue;
		VkSurfaceKHR					m_surface;
		VkSwapchainKHR					m_swapChain;
		VkFormat						m_swapChainFormat;
		VkExtent2D						m_swapChainExtent;
		VkRenderPass					m_renderPass;
		VkPipelineLayout				m_pipelineLayout;
		VkPipeline						m_graphicsPipeline;
		VkCommandPool					m_commandPool;
		VkDescriptorSetLayout			m_descriptorSetLayout;
		VkDescriptorSetLayout			m_textureDescriptorSetLayout;
		VkDescriptorPool				m_descriptorPool;
		DepthStencilBufferVK			m_depthStencilBuffer;
		FrameInFlight					m_framesInFlight[RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT];

		GraphicsPipeline				m_graphicsPipelineSettings;

		uint32 m_currentFrame;

		std::vector<VkCommandBuffer>	m_commandBuffers;
		uint32 m_currentFramebuffer;
		
		ShaderInfoVK*					m_pActiveShader;

		std::vector<VkImage>			m_swapChainImages;
		std::vector<VkImageView>		m_swapChainImageViews;
		std::vector<VkFramebuffer>		m_swapChainFramebuffers;

		std::vector<cstring>			m_validationLayers;
		std::vector<cstring>			m_deviceExtensions;

		std::vector<ShaderInfoVK>		m_loadedShaders;
	};

} }
