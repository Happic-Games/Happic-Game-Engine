#include "RenderContextVK.h"
#include "../../DataStructures/Vector.h"
#include <iostream>
#include "../Windows/Win32Display.h"
#include <vulkan\vulkan_win32.h>
#include "../../Math/GeneralMath.h"
#include "BufferVK.h"
#include <vulkan\spirv.h>
#include "../../File/FileReader.h"
#include <string>
#include "Texture2DVK.h"
#include "TextureSamplerVK.h"
#undef max

#define SPIRV_ROOT_SHADER_PATH Happic::String("Res/Shaders/SPIR-V/")

namespace Happic { namespace Rendering {

	VkBool32 VKAPI_CALL debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	RenderContextVK::RenderContextVK()
	{
		m_currentFramebuffer = 0;
		WindowEventAction windowEventAction;
		windowEventAction.function = window_resize_update_swapchain;
		windowEventAction.pUserPtr = this;

		WindowEventSystem::AddWindowEventAction(WINDOW_EVENT_RESIZE, windowEventAction);

		m_validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");
		m_deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	RenderContextVK::~RenderContextVK()
	{
		CleanupSwapChain();

		vkDestroyDescriptorPool(m_device, m_descriptorPool, NULL);
		vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, NULL);
		vkDestroyDescriptorSetLayout(m_device, m_textureDescriptorSetLayout, NULL);

		for (uint32 i = 0; i < RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT; i++)
		{
			for (uint32 j = 0; j < NUM_SHADER_TYPES; j++)
			{
				delete m_framesInFlight[i].uniformBuffers[j].perWindowResizeBuffer;
				delete m_framesInFlight[i].uniformBuffers[j].perFrameBuffer;
				delete m_framesInFlight[i].uniformBuffers[j].perDrawInstanceBuffer;
			}
		}

		for (uint32 i = 0; i < RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_device, m_framesInFlight[i].imageAvailableSemaphore, NULL);
			vkDestroySemaphore(m_device, m_framesInFlight[i].renderFinishedSemaphore, NULL);
			vkDestroyFence(m_device, m_framesInFlight[i].fence, NULL);
		}

		for (ShaderInfoVK& shaderInfo : m_loadedShaders)
			for (uint32 i = 0; i < NUM_SHADER_TYPES; i++)
				vkDestroyShaderModule(m_device, shaderInfo.shaderModules[i], NULL);

		vkDestroyCommandPool(m_device, m_commandPool, NULL);

		vkDestroyDevice(m_device, NULL);

#ifdef _DEBUG
		auto destroyFunction = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
		destroyFunction(m_instance, m_callback, NULL);
#endif

		vkDestroySurfaceKHR(m_instance, m_surface, NULL);
		vkDestroyInstance(m_instance, NULL);
	}

	void RenderContextVK::Init(const RenderContextInitInfo& initInfo)
	{
		CheckDebugValidationLayers();
		InitInstance();
		InitValidationLayers();
		InitSurface(initInfo.pDisplay);
		InitPhysicalDevice();
		InitQueueFamilyIndices();
		CreateLogicalDevice();
		uint32 width, height;
		initInfo.pDisplay->GetSize(&width, &height);
		InitSwapChain(width, height);
		InitImageViews();
		InitRenderPass();
		InitGraphicsPipeline(initInfo.pipeline);
		InitCommandPool();
		InitCommandBuffers();
		InitSemaphoresAndFences();
		InitUniformBuffers();
		InitDescriptorPool();
		InitDescriptorSets();
		InitDepthStencilBuffer();
		InitFramebuffers();
	}

	VkDescriptorSet RenderContextVK::UpdateSamplerDescriptors(const TextureGroup& textureGroup) const
	{
		const FrameInFlight& frame = m_framesInFlight[m_currentFrame];
		const auto&& index = frame.textureDescriptorSets.find(textureGroup.GetID());
		if (index == frame.textureDescriptorSets.end())
		{
			VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
			descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptor_set_allocate_info.descriptorPool = m_descriptorPool;
			descriptor_set_allocate_info.pSetLayouts = &m_textureDescriptorSetLayout;
			descriptor_set_allocate_info.descriptorSetCount = 1;

			VkDescriptorSet textureDescriptorSet = 0;

			for (uint32 i = 0; i < m_pActiveShader->samplers.size(); i++)
			{
				vkAllocateDescriptorSets(m_device, &descriptor_set_allocate_info, &textureDescriptorSet);

				TextureSamplerPair textureAndSampler = textureGroup.GetTextureAndSampler(m_pActiveShader->samplers[i].name);

				VkDescriptorImageInfo descriptor_image_info{};
				descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				descriptor_image_info.imageView = ((Texture2DVK*)textureAndSampler.pTexture)->GetImageView();
				descriptor_image_info.sampler = ((TextureSamplerVK*)textureAndSampler.pSampler)->GetSampler();

				VkWriteDescriptorSet write_descriptor_set{};
				write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write_descriptor_set.dstSet = textureDescriptorSet;
				write_descriptor_set.dstBinding = m_pActiveShader->samplers[i].binding;
				write_descriptor_set.dstArrayElement = 0;
				write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write_descriptor_set.descriptorCount = 1;
				write_descriptor_set.pImageInfo = &descriptor_image_info;

				vkUpdateDescriptorSets(m_device, 1, &write_descriptor_set, 0, NULL);
				int w = -1;
			}

			frame.textureDescriptorSets[textureGroup.GetID()] = textureDescriptorSet;
			return textureDescriptorSet;
		}
		else
		{
			return index->second;
		}
		
	}

	void RenderContextVK::UpdateUniformBuffer(IBuffer* pBuffer, uint32 offset, const void* pData) const
	{
		void* pBufferData = pBuffer->Map();
		memcpy((unsigned char*)pBufferData + offset, pData, pBuffer->GetSize());
		pBuffer->Unmap();

	}

	VkFormat RenderContextVK::FindSupportedFormats(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties format_properties;
			vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &format_properties);
			if (tiling == VK_IMAGE_TILING_LINEAR && (format_properties.linearTilingFeatures & features) == features)
				return format;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (format_properties.optimalTilingFeatures & features) == features)
				return format;
		}

		return VK_FORMAT_UNDEFINED;
	}

	VkFormat RenderContextVK::FindDepthFormat()
	{
		std::vector<VkFormat> formats = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
		VkFormat format = FindSupportedFormats(formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		return format;
	}

	bool RenderContextVK::HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void RenderContextVK::UpdatePerDrawInstanceBuffer(ShaderType type, const void * pData)
	{
		FrameInFlight& frame = m_framesInFlight[m_currentFrame];
		memcpy((unsigned char*)frame.uniformBuffers[type].pMappedInstanceBufferData + frame.dynamicUniformOffsets[type], pData,
			frame.uniformBuffers[type].instanceAlignment);
	}

	void RenderContextVK::SubmitDrawCommand(const DrawCommand & drawCommand) const
	{
		const FrameInFlight& frame = m_framesInFlight[m_currentFrame];

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(frame.renderCommandBuffer, 0, 1, &((BufferVK*)drawCommand.pVertexBuffer)->GetBuffer(), &offset);
		if (drawCommand.type == DRAW_COMMAND_DRAW_INDEXED)
			vkCmdBindIndexBuffer(frame.renderCommandBuffer, ((BufferVK*)drawCommand.pIndexBuffer)->GetBuffer(), offset, VK_INDEX_TYPE_UINT32);

		for (uint32 i = 0; i < drawCommand.subDraws.size(); i++)
		{
			const SubDraw& subDraw = drawCommand.subDraws[i];
			VkDescriptorSet descriptorSets[2] = { frame.uniformBufferdescriptorSet, UpdateSamplerDescriptors(drawCommand.textureGroups[subDraw.textureGroupIndex]) };
			vkCmdBindDescriptorSets(frame.renderCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 2, descriptorSets,
				frame.dynamicUniformOffsetCount, frame.dynamicUniformOffsets);

			if (drawCommand.type == DRAW_COMMAND_DRAW)
			{
				vkCmdDraw(frame.renderCommandBuffer,subDraw.endIndex - subDraw.startIndex + 1, 1, subDraw.startIndex, 0);
			}
			else if (drawCommand.type == DRAW_COMMAND_DRAW_INDEXED)
			{
				vkCmdDrawIndexed(frame.renderCommandBuffer, subDraw.endIndex - subDraw.startIndex + 1, 1, subDraw.startIndex, 0, 0);
			}
		}

		for(uint32 i = 0; i < NUM_SHADER_TYPES; i++)
			frame.dynamicUniformOffsets[i] += frame.uniformBuffers[i].instanceAlignment;

	}

	void RenderContextVK::BeginFrame() const
	{
		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = m_renderPass;
		render_pass_begin_info.framebuffer = m_swapChainFramebuffers[m_currentFramebuffer];
		render_pass_begin_info.renderArea.offset.x = 0;
		render_pass_begin_info.renderArea.offset.y = 0;
		render_pass_begin_info.renderArea.extent = m_swapChainExtent;

		VkClearValue clear_values[2] = { { 0.0f, 0.0f, 0.0f, 1.0f }, {1.0f, 0.0f, 0.0f, 0.0f} };
		render_pass_begin_info.clearValueCount = 2;
		render_pass_begin_info.pClearValues = clear_values;

		VkCommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		command_buffer_begin_info.pInheritanceInfo = NULL;

		vkWaitForFences(m_device, 1, &m_framesInFlight[m_currentFrame].fence, VK_TRUE, std::numeric_limits<uint64_t>::max()); vkWaitForFences(m_device, 1, &m_framesInFlight[m_currentFrame].fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

		VkResult err = vkBeginCommandBuffer(m_framesInFlight[m_currentFrame].renderCommandBuffer, &command_buffer_begin_info);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error begining Vulkan command buffer : " << std::endl;
			return;
		}

		vkCmdBeginRenderPass(m_framesInFlight[m_currentFrame].renderCommandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(m_framesInFlight[m_currentFrame].renderCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
	}

	void RenderContextVK::Swap()
	{
		vkCmdEndRenderPass(m_framesInFlight[m_currentFrame].renderCommandBuffer);
		vkEndCommandBuffer(m_framesInFlight[m_currentFrame].renderCommandBuffer);

		uint32 imageIndex;
		VkResult err = vkAcquireNextImageKHR(m_device, m_swapChain, std::numeric_limits<uint64_t>::max(),
			m_framesInFlight[m_currentFrame].imageAvailableSemaphore, NULL, &imageIndex);

		if (err == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain(m_swapChainExtent.width, m_swapChainExtent.height);
			return;
		}
		else if (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR)
		{
			std::cerr << "Error acquiring next Vulkan image from swapchain" << std::endl;
			return;
		}

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submit_info {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &m_framesInFlight[m_currentFrame].imageAvailableSemaphore;
		submit_info.pWaitDstStageMask = &waitStage;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &m_framesInFlight[m_currentFrame].renderCommandBuffer;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &m_framesInFlight[m_currentFrame].renderFinishedSemaphore;

		vkResetFences(m_device, 1, &m_framesInFlight[m_currentFrame].fence);

		err = vkQueueSubmit(m_graphicsQueue, 1, &submit_info, m_framesInFlight[m_currentFrame].fence);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error submiting Vulkan graphics queue" << std::endl;
			return;
		}

		VkPresentInfoKHR present_info {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &m_framesInFlight[m_currentFrame].renderFinishedSemaphore;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &m_swapChain;
		present_info.pImageIndices = &imageIndex;
		present_info.pResults = NULL;

		err = vkQueuePresentKHR(m_presentQueue, &present_info);
		if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
		{
			RecreateSwapChain(m_swapChainExtent.width, m_swapChainExtent.height);
		}
		else if (err != VK_SUCCESS)
		{
			std::cerr << "Error acquiring next Vulkan image from swapchain" << std::endl;
		}

		for (uint32 i = 0; i < m_framesInFlight[m_currentFrame].dynamicUniformOffsetCount; i++)
			m_framesInFlight[m_currentFrame].dynamicUniformOffsets[i] = 0;

		m_currentFrame = (m_currentFrame + 1) % RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT;
		m_currentFramebuffer = (m_currentFramebuffer + 1) % 3;
	}

	void RenderContextVK::DisplayClosed() const
	{
		vkDeviceWaitIdle(m_device);
	}

	uint32 RenderContextVK::FindMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties physical_device_memory_properties {};
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &physical_device_memory_properties);

		for (uint32 i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
		{
			if ((typeFilter & (i << i)) && (physical_device_memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		std::cerr << "Could not find suitable memory type" << std::endl;
		return -1;
	}

	void RenderContextVK::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
	{
		VkCommandBuffer copy_command_buffer = BeginSingleTimeCommands();
		VkBufferCopy buffer_copy {};
		buffer_copy.srcOffset = 0;
		buffer_copy.dstOffset = 0;
		buffer_copy.size = size;
		vkCmdCopyBuffer(copy_command_buffer, src, dst, 1, &buffer_copy);

		EndSingleTimeCommands(copy_command_buffer);
	}

	void RenderContextVK::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer transition_image_layout_command_buffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier image_memory_barrier {};
		image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barrier.oldLayout = oldLayout;
		image_memory_barrier.newLayout = newLayout;
		image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.image = image;
		image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_memory_barrier.subresourceRange.baseMipLevel = 0;
		image_memory_barrier.subresourceRange.levelCount = 1;
		image_memory_barrier.subresourceRange.baseArrayLayer = 0;
		image_memory_barrier.subresourceRange.layerCount = 1;
		image_memory_barrier.srcAccessMask = 0; // TODO
		image_memory_barrier.dstAccessMask = 0; // TODO

		VkPipelineStageFlags source_pipeline_stage_flags;
		VkPipelineStageFlags dest_pipeline_stage_flags;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
		{
			image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (HasStencilComponent(format))
				image_memory_barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else
		{
			image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
	

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = 0;
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_pipeline_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dest_pipeline_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_pipeline_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dest_pipeline_stage_flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = 0;
			image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			source_pipeline_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dest_pipeline_stage_flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			std::cerr << "Unsupported layout transition" << std::endl;
		}

		vkCmdPipelineBarrier(transition_image_layout_command_buffer, source_pipeline_stage_flags, dest_pipeline_stage_flags,
			0, 0, NULL, 0, NULL, 1, &image_memory_barrier);

		EndSingleTimeCommands(transition_image_layout_command_buffer);
	}

	void RenderContextVK::CopyBufferToImage(VkBuffer src, VkImage dst, uint32 width, uint32 height)
	{
		VkCommandBuffer copy_buffer_to_image_command_buffer = BeginSingleTimeCommands();

		VkBufferImageCopy buffer_image_copy = {};
		buffer_image_copy.bufferOffset = 0;
		buffer_image_copy.bufferRowLength = 0;
		buffer_image_copy.bufferImageHeight = 0;

		buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		buffer_image_copy.imageSubresource.mipLevel = 0;
		buffer_image_copy.imageSubresource.baseArrayLayer = 0;
		buffer_image_copy.imageSubresource.layerCount = 1;

		buffer_image_copy.imageOffset.x = 0;
		buffer_image_copy.imageOffset.y = 0;
		buffer_image_copy.imageOffset.z = 0;
		buffer_image_copy.imageExtent.width = width;
		buffer_image_copy.imageExtent.height = height;
		buffer_image_copy.imageExtent.depth = 1;

		vkCmdCopyBufferToImage(copy_buffer_to_image_command_buffer, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);

		EndSingleTimeCommands(copy_buffer_to_image_command_buffer);
	}

	bool RenderContextVK::CreateImage(uint32 width, uint32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage * pImage, VkDeviceMemory * pImageMemory)
	{
		VkImageCreateInfo image_create_info{};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.extent.width = width;
		image_create_info.extent.height = height;
		image_create_info.extent.depth = 1;
		image_create_info.mipLevels = 1;
		image_create_info.arrayLayers = 1;
		image_create_info.format = format;
		image_create_info.tiling = tiling;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_create_info.usage = usage;
		image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.flags = 0;

		VkResult err = vkCreateImage(m_device, &image_create_info, NULL, pImage);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan image" << std::endl;
			return false;
		}

		VkMemoryRequirements memory_requirements;
		vkGetImageMemoryRequirements(m_device, *pImage, &memory_requirements);

		VkMemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_allocate_info.memoryTypeIndex = FindMemoryType(memory_requirements.memoryTypeBits, properties);

		err = vkAllocateMemory(m_device, &memory_allocate_info, NULL, pImageMemory);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error allocation Vulkan image memory" << std::endl;
			return false;
		}

		vkBindImageMemory(m_device, *pImage, *pImageMemory, 0);

		return true;
	}

	bool RenderContextVK::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags imageAspectFlags, VkImageView * pImageView)
	{
		VkImageViewCreateInfo image_view_create_info {};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = image;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = format;
		image_view_create_info.subresourceRange.aspectMask = imageAspectFlags;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;

		VkResult err = vkCreateImageView(m_device, &image_view_create_info, NULL, pImageView);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan image view" << std::endl;
			return false;
		}
	}

	const VkDevice& RenderContextVK::GetDevice() const
	{
		return m_device;
	}

	void RenderContextVK::CheckDebugValidationLayers()
	{
#ifndef _DEBUG
		return;
#else

		uint32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, NULL);

		std::vector<VkLayerProperties> properties(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, &properties[0]);

		std::vector<cstring> missingValidationLayers;
		for (cstring layerName : m_validationLayers)
		{
			bool layerFound = false;

			for (VkLayerProperties layerProperties : properties)
			{
				if (strcmp(layerProperties.layerName, layerName) == 0)
				{
					layerFound = true;
				}
			}
			
			if (!layerFound)
			{
				missingValidationLayers.push_back(layerName);
			}
		}

		if (!missingValidationLayers.empty())
		{
			std::cout << "Missing some Vulkan validation layers :" << std::endl;

			for (cstring name : missingValidationLayers)
			{
				std::cout << "\t" << name << std::endl;
			}
		}
#endif
	}

	void RenderContextVK::InitInstance()
	{
		VkApplicationInfo application_info{};
		application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info.pApplicationName = "Game";
		application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info.pEngineName = "Happic";
		application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info.apiVersion = VK_MAKE_VERSION(1, 1, 0);

		std::vector<cstring> requiredExtensions = GetRequiredExtensions();

		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo = &application_info;
		instance_create_info.enabledExtensionCount = requiredExtensions.size();
		instance_create_info.ppEnabledExtensionNames = &requiredExtensions[0];

#ifdef _DEBUG
		instance_create_info.enabledLayerCount = m_validationLayers.size();
		instance_create_info.ppEnabledLayerNames = &m_validationLayers[0];
#else
		instance_create_info.enabledLayerCount = 0;
#endif

		VkResult err = vkCreateInstance(&instance_create_info, NULL, &m_instance);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan instance" << std::endl;
			return;
		}
	}

	std::vector<cstring> RenderContextVK::GetRequiredExtensions() const
	{
		std::vector<cstring> requiredExtensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };
#ifdef _DEBUG
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		uint32 extensionCount;
		vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, &availableExtensions[0]);

		std::vector<cstring> missingExtensions;
		for (cstring requiredExtension : requiredExtensions)
		{
			bool found = false;

			for (VkExtensionProperties availableExtension : availableExtensions)
			{
				if (strcmp(requiredExtension, availableExtension.extensionName) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				missingExtensions.push_back(requiredExtension);
			}
		}

		if (!missingExtensions.empty())
		{
			std::cerr << "Missing required Vulkan layer extensions :" << std::endl;
			for (uint32 i = 0; i < missingExtensions.size(); i++)
			{
				std::cout << "\t" << missingExtensions[i] << std::endl;
			}
		}

		return requiredExtensions;
	}

	void RenderContextVK::InitValidationLayers()
	{
#ifndef _DEBUG
		return;
#else
		VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info {};

		debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		debug_utils_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		debug_utils_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		debug_utils_messenger_create_info.pfnUserCallback = debug_callback;
		debug_utils_messenger_create_info.pUserData = NULL;

		auto createFunction = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
		if (createFunction)
		{
			VkResult err = createFunction(m_instance, &debug_utils_messenger_create_info, NULL, &m_callback);
			if (err != VK_SUCCESS)
			{
				std::cerr << "Error creating Vulkan debug utils messenger" << std::endl;
			}
		}
		else
		{
			std::cerr << "Error creating Vulkan debug utils messenger" << std::endl;
		}
#endif

	}

	void RenderContextVK::InitSurface(IDisplay* pDisplay)
	{
		Win32Display* pWin32Display = (Win32Display*)pDisplay;

		VkWin32SurfaceCreateInfoKHR win32_surface_create_info {};
		win32_surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		win32_surface_create_info.hwnd = pWin32Display->GetWindowHandle();
		//win32_surface_create_info.hinstance = (HINSTANCE)GetWindowLong(pWin32Display->GetWindowHandle(), GWL_HINSTANCE);
		win32_surface_create_info.hinstance = GetModuleHandle(NULL);

		vkCreateWin32SurfaceKHR(m_instance, &win32_surface_create_info, NULL, &m_surface);
	}

	void RenderContextVK::InitPhysicalDevice()
	{
		uint32 physicalDeviceCount;
		vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, NULL);

		if (physicalDeviceCount == 0)
		{
			std::cerr << "Could not find a GPU with Vulkan support" << std::endl;
			return;
		}

		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, &physicalDevices[0]);

		for (VkPhysicalDevice device : physicalDevices)
		{
			if (IsDeviceSuitable(device))
			{
				m_physicalDevice = device;
				break;
			}
		}
	}

	bool RenderContextVK::IsDeviceSuitable(VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(physicalDevice, &features);

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		bool extensionSupported = CheckDeviceExtensionSupport(physicalDevice);
		bool swapChainAdequate = false;
		if (extensionSupported)
		{
			SwapChainSupportDetails details = QuerySwapChainSupportDetails(physicalDevice);
			swapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
		}

		bool suitable = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && extensionSupported && swapChainAdequate &&
			features.samplerAnisotropy;

		return suitable;
	}

	bool RenderContextVK::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
	{
		uint32 extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extensionCount, NULL);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extensionCount, &extensions[0]);

		std::vector<cstring> missingExtensions;
		for (cstring requiredExtension : m_deviceExtensions)
		{
			bool extensionFound = false;

			for (const VkExtensionProperties& extension : extensions)
			{
				if (strcmp(requiredExtension, extension.extensionName) == 0)
				{
					extensionFound = true;
				}
			}

			if (!extensionFound)
			{
				missingExtensions.push_back(requiredExtension);
			}
		}

		if (!missingExtensions.empty())
		{
			std::cout << "Missing some Vulkan device extensions :" << std::endl;

			for (cstring name : missingExtensions)
			{
				std::cout << "\t" << name << std::endl;
			}

			return false;
		}

		return true;
	}

	void RenderContextVK::InitQueueFamilyIndices()
	{
		uint32 queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, NULL);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, &queueFamilies[0]);

		m_queueFamilies.graphics = -1;
		m_queueFamilies.present = -1;

		for (uint32 i = 0; i < queueFamilies.size(); i++)
		{
			const VkQueueFamilyProperties& properties = queueFamilies[i];

			if (properties.queueCount > 0 && (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) && m_queueFamilies.graphics == -1)
			{
				m_queueFamilies.graphics = i;
			}

			VkBool32 presentSupport = false;
			VkResult err = vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);

			if (queueFamilies[i].queueCount > 0 && presentSupport && m_queueFamilies.present == -1)
			{
				m_queueFamilies.present = i;
			}
		}
	}

	void RenderContextVK::CreateLogicalDevice()
	{
		VkPhysicalDeviceFeatures enabledFeatures{};
		enabledFeatures.samplerAnisotropy = VK_TRUE;

		std::vector<int32> queueFamilies = { m_queueFamilies.graphics };

		if (m_queueFamilies.present != m_queueFamilies.graphics)
		{
			queueFamilies.push_back(m_queueFamilies.present);
		}

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		float queuePriority = 1.0f;
		for (int32 queueFamily : queueFamilies)
		{
			VkDeviceQueueCreateInfo device_queue_create_info{};
			device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			device_queue_create_info.queueFamilyIndex = queueFamily;
			device_queue_create_info.queueCount = 1;
			device_queue_create_info.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(device_queue_create_info);
		}

		VkDeviceCreateInfo device_create_info {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.pQueueCreateInfos = &queueCreateInfos[0];
		device_create_info.queueCreateInfoCount = queueCreateInfos.size();
		device_create_info.pEnabledFeatures = &enabledFeatures;
		device_create_info.enabledExtensionCount = m_deviceExtensions.size();
		device_create_info.ppEnabledExtensionNames = &m_deviceExtensions[0];
#ifdef _DEBUG
		device_create_info.enabledLayerCount = m_validationLayers.size();
		device_create_info.ppEnabledLayerNames = &m_validationLayers[0];
#else
		device_create_info.enabledLayerCount = 0;
#endif

		VkResult err = vkCreateDevice(m_physicalDevice, &device_create_info, NULL, &m_device);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan logical device" << std::endl;
			return;
		}

		vkGetDeviceQueue(m_device, m_queueFamilies.graphics, 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, m_queueFamilies.present, 0, &m_presentQueue);
	}

	SwapChainSupportDetails RenderContextVK::QuerySwapChainSupportDetails(VkPhysicalDevice physicalDevice)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &details.capabilities);

		uint32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, NULL);

		if (formatCount == 0)
		{
			std::cerr << "No Vulkan surface formats" << std::endl;
			return details;
		}

		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, &details.formats[0]);

		uint32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, NULL);

		if (presentModeCount == 0)
		{
			std::cerr << "No Vulkan surface present modes" << std::endl;
			return details;
		}

		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, &details.presentModes[0]);

		

		return details;
	}

	VkSurfaceFormatKHR RenderContextVK::ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
	{
		if (formats.size() == 1 && formats.at(0).format == VK_FORMAT_UNDEFINED)
		{
			VkSurfaceFormatKHR format;
			format.format = VK_FORMAT_R8G8B8A8_UNORM;
			format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			return format;
		}

		for (const VkSurfaceFormatKHR& format : formats)
		{
			if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return format;
		}

		return formats[0];
	}

	VkPresentModeKHR RenderContextVK::ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
	{
		for (const VkPresentModeKHR& presentMode : presentModes)
		{
			if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
				return presentMode;
		}

		for (const VkPresentModeKHR& presentMode : presentModes)
		{
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return presentMode;
		}

		return presentModes[0];
	}

	VkExtent2D RenderContextVK::ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR & capabilities, uint32 width, uint32 height)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32>::max())
		{
			return capabilities.currentExtent;
		}

		VkExtent2D extent;
		extent.width = width;
		extent.height = height;

		extent.width = MATH_CLAMP(extent.width, capabilities.maxImageExtent.width, capabilities.minImageExtent.width);
		extent.height = MATH_CLAMP(extent.height, capabilities.maxImageExtent.height, capabilities.minImageExtent.height);

		return extent;
	}

	void RenderContextVK::InitSwapChain(uint32 width, uint32 height)
	{
		SwapChainSupportDetails swapChainSupportDetails = QuerySwapChainSupportDetails(m_physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapChainSurfaceFormat(swapChainSupportDetails.formats);
		VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(swapChainSupportDetails.presentModes);
		VkExtent2D extent = ChooseSwapChainExtent(swapChainSupportDetails.capabilities, width, height);

		m_swapChainFormat = surfaceFormat.format;
		m_swapChainExtent = extent;

		unsigned int imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
		if (swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount)
		{
			imageCount = swapChainSupportDetails.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.surface = m_surface;
		swapchain_create_info.minImageCount = imageCount;
		swapchain_create_info.imageFormat = surfaceFormat.format;
		swapchain_create_info.imageColorSpace = surfaceFormat.colorSpace;
		swapchain_create_info.imageExtent = extent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.preTransform = swapChainSupportDetails.capabilities.currentTransform;
		swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.presentMode = presentMode;
		swapchain_create_info.clipped = VK_TRUE;
		swapchain_create_info.oldSwapchain = NULL;

		if (m_queueFamilies.graphics == m_queueFamilies.present)
		{
			swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchain_create_info.queueFamilyIndexCount = 0;
			swapchain_create_info.pQueueFamilyIndices = NULL;
		}
		else
		{
			uint32 queueFamilies[2] = { m_queueFamilies.graphics, m_queueFamilies.present };
			swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchain_create_info.queueFamilyIndexCount = 2;
			swapchain_create_info.pQueueFamilyIndices = queueFamilies;
		}

		VkResult err = vkCreateSwapchainKHR(m_device, &swapchain_create_info, NULL, &m_swapChain);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Could not create Vulkan swapchain" << std::endl;
			return;
		}

		uint32 swapChainImageCount;
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapChainImageCount, NULL);

		m_swapChainImages.resize(swapChainImageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapChainImageCount, &m_swapChainImages[0]);
	}

	void RenderContextVK::InitImageViews()
	{
		m_swapChainImageViews.resize(m_swapChainImages.size());

		for (uint32 i = 0; i < m_swapChainImages.size(); i++)
		{
			VkImageViewCreateInfo image_view_create_info {};
			image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.image = m_swapChainImages[i];
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format = m_swapChainFormat;

			image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.baseMipLevel = 0;
			image_view_create_info.subresourceRange.levelCount = 1;
			image_view_create_info.subresourceRange.baseArrayLayer = 0;
			image_view_create_info.subresourceRange.layerCount = 1;

			VkResult err = vkCreateImageView(m_device, &image_view_create_info, NULL, &m_swapChainImageViews[i]);
			if (err != VK_SUCCESS)
			{
				std::cerr << "Error creating Vulkan image view" << std::endl;
				return;
			}
		}
	}

	void RenderContextVK::InitRenderPass()
	{
		VkAttachmentDescription color_attachment_description{};
		color_attachment_description.format = m_swapChainFormat;
		color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depth_attachment_description{};
		depth_attachment_description.format = FindDepthFormat();
		depth_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_attachment_reference {};
		color_attachment_reference.attachment = 0;
		color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_reference {};
		depth_attachment_reference.attachment = 1;
		depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description{};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &color_attachment_reference;
		subpass_description.pDepthStencilAttachment = &depth_attachment_reference;

		VkAttachmentDescription attachment_descriptions[2] = { color_attachment_description, depth_attachment_description };

		VkRenderPassCreateInfo render_pass_create_info {};
		render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.attachmentCount = 2;
		render_pass_create_info.pAttachments = attachment_descriptions;
		render_pass_create_info.subpassCount = 1;
		render_pass_create_info.pSubpasses = &subpass_description;

		VkSubpassDependency subpass_depencency{};
		subpass_depencency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpass_depencency.dstSubpass = 0;
		subpass_depencency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_depencency.srcAccessMask = 0;
		subpass_depencency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_depencency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		render_pass_create_info.dependencyCount = 1;
		render_pass_create_info.pDependencies = &subpass_depencency;

		VkResult err = vkCreateRenderPass(m_device, &render_pass_create_info, NULL, &m_renderPass);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan render pass" << std::endl;
			return;
		}
	}

	void RenderContextVK::InitGraphicsPipeline(const GraphicsPipeline & pipeline)
	{
		uint32 shaderID = -1;
		if (m_graphicsPipelineSettings.shaderInfo.shaderPaths[SHADER_TYPE_VERTEX].IsEmpty())
		{
			m_graphicsPipelineSettings = pipeline;

			shaderID = LoadShader(pipeline.shaderInfo, m_device);
			m_pActiveShader = &m_loadedShaders[shaderID];
		}

		std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;
		for (uint32 i = 0; i < m_pActiveShader->shaderBuffers.size(); i++)
		{
			VkDescriptorSetLayoutBinding descriptor_set_layout_binding {};
			descriptor_set_layout_binding.binding = m_pActiveShader->shaderBuffers[i].binding;

			if(m_pActiveShader->shaderBuffers[i].name == "PerDrawInstance")
				descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			else
				descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

			descriptor_set_layout_binding.descriptorCount = 1;

			VkShaderStageFlags shader_stage_flags = 0;
			if (m_pActiveShader->shaderBuffers[i].shaderStage == SHADER_TYPE_VERTEX)
				shader_stage_flags = VK_SHADER_STAGE_VERTEX_BIT;
			else if (m_pActiveShader->shaderBuffers[i].shaderStage == SHADER_TYPE_FRAGMENT)
				shader_stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT;

			descriptor_set_layout_binding.stageFlags = shader_stage_flags;
			descriptor_set_layout_binding.pImmutableSamplers = NULL;
			descriptor_set_layout_bindings.push_back(descriptor_set_layout_binding);
		}

		std::vector<VkDescriptorSetLayoutBinding> texture_descriptor_set_layout_bindings;
		for (uint32 i = 0; i < m_pActiveShader->samplers.size(); i++)
		{
			VkDescriptorSetLayoutBinding texture_descriptor_set_layout_binding{};
			texture_descriptor_set_layout_binding.binding = m_pActiveShader->samplers[i].binding;
			texture_descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			texture_descriptor_set_layout_binding.descriptorCount = 1;
			texture_descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			texture_descriptor_set_layout_binding.pImmutableSamplers = NULL;
			texture_descriptor_set_layout_bindings.push_back(texture_descriptor_set_layout_binding);
		}

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.bindingCount = descriptor_set_layout_bindings.size();
		descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_bindings[0];

		VkDescriptorSetLayoutCreateInfo texture_descriptor_set_layout_create_info{};
		texture_descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		texture_descriptor_set_layout_create_info.bindingCount = texture_descriptor_set_layout_bindings.size();
		texture_descriptor_set_layout_create_info.pBindings = &texture_descriptor_set_layout_bindings[0];

		VkResult err  = vkCreateDescriptorSetLayout(m_device, &descriptor_set_layout_create_info, NULL, &m_descriptorSetLayout);
		VkResult err2 = vkCreateDescriptorSetLayout(m_device, &texture_descriptor_set_layout_create_info, NULL, &m_textureDescriptorSetLayout);

		if (err != VK_SUCCESS || err2 != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan descriptor set" << std::endl;
			return;
		}

		VkVertexInputBindingDescription vertex_input_binding_description {};
		vertex_input_binding_description.binding = 0;
		vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertex_input_binding_description.stride = pipeline.inputLayout.vertexSize;

		std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions(pipeline.inputLayout.numVertexAttributes);
		for (uint32 i = 0; i < vertex_input_attribute_descriptions.size(); i++)
		{
			VkVertexInputAttributeDescription vertex_input_attribute_description {};
			vertex_input_attribute_description.binding = 0;
			vertex_input_attribute_description.location = pipeline.inputLayout.vertexAttributes[i].location;
			vertex_input_attribute_description.offset = pipeline.inputLayout.vertexAttributes[i].offset;

			switch (pipeline.inputLayout.vertexAttributes[i].type)
			{
			case VERTEX_ATTRIBUTE_FLOAT:  vertex_input_attribute_description.format = VK_FORMAT_R32_SFLOAT;			 break;
			case VERTEX_ATTRIBUTE_FLOAT2: vertex_input_attribute_description.format = VK_FORMAT_R32G32_SFLOAT;		 break;
			case VERTEX_ATTRIBUTE_FLOAT3: vertex_input_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;	 break;
			case VERTEX_ATTRIBUTE_FLOAT4: vertex_input_attribute_description.format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
			}

			vertex_input_attribute_descriptions[i] = vertex_input_attribute_description;
		}


		VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info {};
		pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
		pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_input_binding_description;
		pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_input_attribute_descriptions.size();
		pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = &vertex_input_attribute_descriptions[0];

		VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info {};
		pipeline_input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipeline_input_assembly_state_create_info.topology = GetTopology(pipeline.primitiveTopology);
		pipeline_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport;
		viewport.x = pipeline.viewport.x;
		viewport.y = pipeline.viewport.y;
		viewport.width = pipeline.viewport.width;
		viewport.height = pipeline.viewport.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor;
		scissor.offset.x = pipeline.scissor.x;
		scissor.offset.y = pipeline.scissor.y;
		scissor.extent.width = pipeline.scissor.width;
		scissor.extent.height = pipeline.scissor.height;

		VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info {};
		pipeline_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipeline_viewport_state_create_info.pScissors = &scissor;
		pipeline_viewport_state_create_info.scissorCount = 1;
		pipeline_viewport_state_create_info.pViewports = &viewport;
		pipeline_viewport_state_create_info.viewportCount = 1;
	
		VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info{ };
		pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipeline_rasterization_state_create_info.depthBiasEnable = VK_FALSE;
		pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
		pipeline_rasterization_state_create_info.lineWidth = 1.0f;
		pipeline_rasterization_state_create_info.polygonMode = GetPolygonMode(pipeline.rasterizerState.polygonMode);
		pipeline_rasterization_state_create_info.cullMode = GetCullMode(pipeline.rasterizerState.cullMode);
		pipeline_rasterization_state_create_info.frontFace = GetFrontFace(pipeline.rasterizerState.frontFace);
		pipeline_rasterization_state_create_info.depthBiasEnable = VK_FALSE;
		pipeline_rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
		pipeline_rasterization_state_create_info.depthBiasClamp = 0.0f;
		pipeline_rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info {};
		pipeline_multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipeline_multisample_state_create_info.sampleShadingEnable = VK_FALSE;
		pipeline_multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipeline_multisample_state_create_info.minSampleShading = 1.0f;
		pipeline_multisample_state_create_info.pSampleMask = NULL;
		pipeline_multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
		pipeline_multisample_state_create_info.alphaToOneEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info{};
		pipeline_depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipeline_depth_stencil_state_create_info.minDepthBounds = 0.0f;
		pipeline_depth_stencil_state_create_info.maxDepthBounds = 1.0f;
		pipeline_depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
		pipeline_depth_stencil_state_create_info.flags = 0;
		pipeline_depth_stencil_state_create_info.depthCompareOp = GetComparison(pipeline.depthStencilState.depthComparison);
		pipeline_depth_stencil_state_create_info.stencilTestEnable = pipeline.depthStencilState.stencilTestEnabled;
		if (pipeline.depthStencilState.stencilTestEnabled)
		{
			pipeline_depth_stencil_state_create_info.front = GetStencilFace(pipeline.depthStencilState.front);
			pipeline_depth_stencil_state_create_info.back = GetStencilFace(pipeline.depthStencilState.back);
		}
		else
		{
			pipeline_depth_stencil_state_create_info.front = {};
			pipeline_depth_stencil_state_create_info.back = {};
		}
		pipeline_depth_stencil_state_create_info.depthTestEnable = pipeline.depthStencilState.depthTestEnabled;
		pipeline_depth_stencil_state_create_info.depthWriteEnable = pipeline.depthStencilState.depthWriteEnabled;
		
		VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state {};
		pipeline_color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		pipeline_color_blend_attachment_state.blendEnable = pipeline.colorBlendState.enabled;
		pipeline_color_blend_attachment_state.alphaBlendOp = GetBlendOperation(pipeline.colorBlendState.alphaBlendOperation);
		pipeline_color_blend_attachment_state.colorBlendOp = GetBlendOperation(pipeline.colorBlendState.operation);
		pipeline_color_blend_attachment_state.dstAlphaBlendFactor = GetBlendFactor(pipeline.colorBlendState.alphaDestFactor);
		pipeline_color_blend_attachment_state.dstColorBlendFactor = GetBlendFactor(pipeline.colorBlendState.destFactor);
		pipeline_color_blend_attachment_state.srcAlphaBlendFactor = GetBlendFactor(pipeline.colorBlendState.alphaSrcFactor);
		pipeline_color_blend_attachment_state.srcColorBlendFactor = GetBlendFactor(pipeline.colorBlendState.srcFactor);

		VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info {};
		pipeline_color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipeline_color_blend_state_create_info.logicOpEnable = VK_FALSE;
		pipeline_color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
		pipeline_color_blend_state_create_info.pAttachments = &pipeline_color_blend_attachment_state;
		pipeline_color_blend_state_create_info.attachmentCount = 1;
		pipeline_color_blend_state_create_info.blendConstants[0] = 0.0f;
		pipeline_color_blend_state_create_info.blendConstants[1] = 0.0f;
		pipeline_color_blend_state_create_info.blendConstants[2] = 0.0f;
		pipeline_color_blend_state_create_info.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates;
		if (pipeline.dynamicState.viewport)
			dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		if (pipeline.dynamicState.scissor)
			dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
		if (pipeline.dynamicState.blendState)
			dynamicStates.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);

		VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info {};
		pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

		if (!dynamicStates.empty())
		{
			pipeline_dynamic_state_create_info.dynamicStateCount = dynamicStates.size();
			pipeline_dynamic_state_create_info.pDynamicStates = &dynamicStates[0];
		}

		VkDescriptorSetLayout layouts[2] = { m_descriptorSetLayout , m_textureDescriptorSetLayout };

		VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount = 2;
		pipeline_layout_create_info.pSetLayouts = layouts;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		pipeline_layout_create_info.pPushConstantRanges = NULL;

		err = vkCreatePipelineLayout(m_device, &pipeline_layout_create_info, NULL, &m_pipelineLayout);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan pipeline layout" << std::endl;
			return;
		}

		VkGraphicsPipelineCreateInfo graphics_pipeline_create_info {};
		graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphics_pipeline_create_info.stageCount = NUM_SHADER_TYPES;
		graphics_pipeline_create_info.pStages = m_pActiveShader->shaderStages;
		graphics_pipeline_create_info.pVertexInputState = &pipeline_vertex_input_state_create_info;
		graphics_pipeline_create_info.pInputAssemblyState = &pipeline_input_assembly_state_create_info;
		graphics_pipeline_create_info.pViewportState = &pipeline_viewport_state_create_info;
		graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
		graphics_pipeline_create_info.pMultisampleState = &pipeline_multisample_state_create_info;
		graphics_pipeline_create_info.pDepthStencilState = &pipeline_depth_stencil_state_create_info;
		graphics_pipeline_create_info.pColorBlendState = &pipeline_color_blend_state_create_info;
		graphics_pipeline_create_info.layout = m_pipelineLayout;
		graphics_pipeline_create_info.renderPass = m_renderPass;
		graphics_pipeline_create_info.subpass = 0;
		graphics_pipeline_create_info.basePipelineHandle = NULL;

		if (!dynamicStates.empty())
		{
			graphics_pipeline_create_info.pDynamicState = &pipeline_dynamic_state_create_info;
		}

		err = vkCreateGraphicsPipelines(m_device, NULL, 1, &graphics_pipeline_create_info, NULL, &m_graphicsPipeline);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan graphics pipeline" << std::endl;
			return;
		}
	}

	void RenderContextVK::InitFramebuffers()
	{
		m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

		for (uint32 i = 0; i < m_swapChainImageViews.size(); i++)
		{
			VkImageView attachments[2] = { m_swapChainImageViews[i], m_depthStencilBuffer.depthStencilImageView };

			VkFramebufferCreateInfo framebuffer_create_info {};
			framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_create_info.renderPass = m_renderPass;
			framebuffer_create_info.attachmentCount = 2;
			framebuffer_create_info.pAttachments = attachments;
			framebuffer_create_info.width = m_swapChainExtent.width;
			framebuffer_create_info.height = m_swapChainExtent.height;
			framebuffer_create_info.layers = 1;

			VkResult err = vkCreateFramebuffer(m_device, &framebuffer_create_info, NULL, &m_swapChainFramebuffers[i]);
			if (err != VK_SUCCESS)
			{
				std::cerr << "Error creating Vulkan framebuffer" << std::endl;
				return;
			}
		}
	}

	void RenderContextVK::InitCommandPool()
	{
		VkCommandPoolCreateInfo command_pool_create_info {};
		command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.queueFamilyIndex = m_queueFamilies.graphics;
		command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		VkResult err = vkCreateCommandPool(m_device, &command_pool_create_info, NULL, &m_commandPool);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan command pool" << std::endl;
			return;
		}
	}

	void RenderContextVK::InitCommandBuffers()
	{
		VkCommandBufferAllocateInfo command_buffer_allocation_info{};
		command_buffer_allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocation_info.commandPool = m_commandPool;
		command_buffer_allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocation_info.commandBufferCount = 1;

		for (uint32 i = 0; i < RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkResult err = vkAllocateCommandBuffers(m_device, &command_buffer_allocation_info, &m_framesInFlight[i].renderCommandBuffer);

			if (err != VK_SUCCESS)
			{
				std::cerr << "Error creating Vulkan command buffers" << std::endl;
				return;
			}
		}
	}

	void RenderContextVK::InitSemaphoresAndFences()
	{
		VkSemaphoreCreateInfo semaphore_create_info {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
		VkFenceCreateInfo fence_create_info {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32 i = 0; i < RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkResult err1 = vkCreateSemaphore(m_device, &semaphore_create_info, NULL, &m_framesInFlight[i].imageAvailableSemaphore);
			VkResult err2 = vkCreateSemaphore(m_device, &semaphore_create_info, NULL, &m_framesInFlight[i].renderFinishedSemaphore);

			if (err1 != VK_SUCCESS || err2 != VK_SUCCESS)
			{
				std::cerr << "Error creating Vulkan semaphores : " << i << std::endl;
				return;
			}

			err1 = vkCreateFence(m_device, &fence_create_info, NULL, &m_framesInFlight[i].fence);
			if (err1 != VK_SUCCESS)
			{
				std::cerr << "Error creating Vulkan fences : " << i << std::endl;
				return;
			}
		}

	}

	void RenderContextVK::InitUniformBuffers()
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
		uint32 minAlignment = properties.limits.minUniformBufferOffsetAlignment;


		for (uint32 i = 0; i < RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT; i++)
		{
			FrameInFlight& frame = m_framesInFlight[i];

			for (uint32 j = 0; j < m_pActiveShader->shaderBuffers.size(); j++)
			{
				const ShaderBufferInfoVK& bufferInfo = m_pActiveShader->shaderBuffers[j];
				ShaderUniformBuffersVK& uniformBuffers = frame.uniformBuffers[bufferInfo.shaderStage];
			
				uint32 dynamicAlignment = bufferInfo.size;
				if (minAlignment > 0)
					dynamicAlignment = (dynamicAlignment + minAlignment - 1) & ~(minAlignment - 1);

				uniformBuffers.instanceAlignment = dynamicAlignment;

				if (m_pActiveShader->shaderBuffers[j].name == "PerWindowResize")
				{
					if (uniformBuffers.perWindowResizeBuffer)
					{
						std::cerr << "A per window resize uniform buffer was already created" << std::endl;
						return;
					}

					uniformBuffers.perWindowResizeBufferBinding = bufferInfo.binding;
					uniformBuffers.perWindowResizeBuffer = 
						(BufferVK*) IBuffer::CreateBuffer(BUFFER_TYPE_UNIFORM, BUFFER_USAGE_DYNAMIC, NULL, bufferInfo.size);
				}
				else if (m_pActiveShader->shaderBuffers[j].name == "PerFrame")
				{
					if (uniformBuffers.perFrameBuffer)
					{
						std::cerr << "A per frame buffer was already created" << std::endl;
						return;
					}

					uniformBuffers.perFrameBufferBinding = bufferInfo.binding;
					uniformBuffers.perFrameBuffer =
						(BufferVK*)IBuffer::CreateBuffer(BUFFER_TYPE_UNIFORM, BUFFER_USAGE_DYNAMIC, NULL, bufferInfo.size);
				}
				else if (m_pActiveShader->shaderBuffers[j].name == "PerDrawInstance")
				{
					if (uniformBuffers.perDrawInstanceBuffer)
					{
						std::cerr << "A per draw instance uniform buffer was already created" << std::endl;
						return;
					}

					uniformBuffers.perDrawInstanceBufferBinding = bufferInfo.binding;
					uniformBuffers.perDrawInstanceBuffer =
						(BufferVK*)IBuffer::CreateBuffer(BUFFER_TYPE_UNIFORM, BUFFER_USAGE_DYNAMIC, NULL,
							RENDER_CONTEXT_VK_MAX_INSTANCES_PER_BUFFER * minAlignment);

					uniformBuffers.pMappedInstanceBufferData = uniformBuffers.perDrawInstanceBuffer->Map();
					frame.dynamicUniformOffsetCount++;
				}
			}
		}
	}

	void RenderContextVK::InitDescriptorPool()
	{
		VkDescriptorPoolSize descriptor_pool_sizes[3];

		descriptor_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_pool_sizes[0].descriptorCount = 2 * RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT * NUM_SHADER_TYPES;

		descriptor_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descriptor_pool_sizes[1].descriptorCount = 1 * RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT * NUM_SHADER_TYPES;

		descriptor_pool_sizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_pool_sizes[2].descriptorCount = m_pActiveShader->samplers.size() * RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT * RENDER_CONTEXT_VK_MAX_INSTANCES_PER_BUFFER;

		VkDescriptorPoolCreateInfo descriptor_pool_create_info {};
		descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_create_info.poolSizeCount = 3;
		descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes;
		descriptor_pool_create_info.maxSets = RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT + RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT * RENDER_CONTEXT_VK_MAX_INSTANCES_PER_BUFFER;

		VkResult err = vkCreateDescriptorPool(m_device, &descriptor_pool_create_info, NULL, &m_descriptorPool);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan descriptor pool" << std::endl;
			return;
		}


	}

	void RenderContextVK::InitDescriptorSets()
	{
		VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
		descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptor_set_allocate_info.descriptorPool = m_descriptorPool;
		descriptor_set_allocate_info.pSetLayouts = &m_descriptorSetLayout;
		descriptor_set_allocate_info.descriptorSetCount = 1;

		for (uint32 i = 0; i < RENDER_CONTEXT_VK_MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkResult err = vkAllocateDescriptorSets(m_device, &descriptor_set_allocate_info, &m_framesInFlight[i].uniformBufferdescriptorSet);

			if (err != VK_SUCCESS)
			{
				std::cerr << "Error allocating Vulkan descriptor set" << std::endl;
				return;
			}

			for (uint32 j = 0; j < NUM_SHADER_TYPES; j++)
			{
				UpdateDescriptorSet(m_framesInFlight[i].uniformBufferdescriptorSet, m_framesInFlight[i].uniformBuffers[j].perWindowResizeBuffer,
					m_framesInFlight[i].uniformBuffers[j].perWindowResizeBufferBinding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

				UpdateDescriptorSet(m_framesInFlight[i].uniformBufferdescriptorSet, m_framesInFlight[i].uniformBuffers[j].perFrameBuffer,
					m_framesInFlight[i].uniformBuffers[j].perFrameBufferBinding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

				UpdateDescriptorSet(m_framesInFlight[i].uniformBufferdescriptorSet, m_framesInFlight[i].uniformBuffers[j].perDrawInstanceBuffer,
					m_framesInFlight[i].uniformBuffers[j].perDrawInstanceBufferBinding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
			}
		}
	}

	void RenderContextVK::UpdateDescriptorSet(VkDescriptorSet descriptorSet, const BufferVK* pBuffer, uint32 binding, VkDescriptorType type)
	{
		if (!pBuffer)
			return;

		VkDescriptorBufferInfo descriptor_buffer_info{};
		descriptor_buffer_info.buffer = pBuffer->GetBuffer();
		descriptor_buffer_info.offset = 0;
		descriptor_buffer_info.range = pBuffer->GetSize();

		VkWriteDescriptorSet write_descriptor_set{};
		write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_set.dstSet = descriptorSet;
		write_descriptor_set.dstBinding = binding;
		write_descriptor_set.dstArrayElement = 0;
		write_descriptor_set.descriptorType = type;
		write_descriptor_set.descriptorCount = 1;
		write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
		write_descriptor_set.pImageInfo = NULL;
		write_descriptor_set.pTexelBufferView = NULL;

		vkUpdateDescriptorSets(m_device, 1, &write_descriptor_set, 0, NULL);
	}

	void RenderContextVK::InitDepthStencilBuffer()
	{
		VkFormat depthFormat = FindDepthFormat();
		if (depthFormat == VK_FORMAT_UNDEFINED)
		{
			std::cerr << "Could not find a supported depth format" << std::endl;
			return;
		}

		bool success = CreateImage(m_swapChainExtent.width, m_swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_depthStencilBuffer.depthStencilImage,
			&m_depthStencilBuffer.imageMemory);

		if (!success)
			return;

		success = CreateImageView(m_depthStencilBuffer.depthStencilImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT,
			&m_depthStencilBuffer.depthStencilImageView);

		if (!success)
			return;

		TransitionImageLayout(m_depthStencilBuffer.depthStencilImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, 
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	}

	void RenderContextVK::CleanupSwapChain()
	{
		vkDestroyImageView(m_device, m_depthStencilBuffer.depthStencilImageView, NULL);
		vkDestroyImage(m_device, m_depthStencilBuffer.depthStencilImage, NULL);
		vkFreeMemory(m_device, m_depthStencilBuffer.imageMemory, NULL);

		for (size_t i = 0; i < m_swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(m_device, m_swapChainFramebuffers[i], nullptr);
		}

		vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

		vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
		vkDestroyRenderPass(m_device, m_renderPass, nullptr);

		for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
			vkDestroyImageView(m_device, m_swapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

		m_commandBuffers.clear();
		m_swapChainImages.clear();
		m_swapChainImageViews.clear();
		m_swapChainFramebuffers.clear();
	}

	void RenderContextVK::RecreateSwapChain(uint32 width, uint32 height)
	{
		vkDeviceWaitIdle(m_device);

		CleanupSwapChain();

		InitSwapChain(width, height);
		InitImageViews();
		InitRenderPass();
		InitGraphicsPipeline(m_graphicsPipelineSettings);
		InitDepthStencilBuffer();
		InitFramebuffers();
		InitCommandBuffers();
	}

	VkCommandBuffer RenderContextVK::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo command_buffer_allocate_info {};
		command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandPool = m_commandPool;
		command_buffer_allocate_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(m_device, &command_buffer_allocate_info, &command_buffer);

		VkCommandBufferBeginInfo command_buffer_begin_info = {};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

		return command_buffer;
	}

	void RenderContextVK::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_graphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_graphicsQueue);

		vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
	}

	uint32 RenderContextVK::LoadShader(const ShaderInfo & shaderInfo, const VkDevice& device)
	{
		ShaderInfoVK shaderInfoVK;
		shaderInfoVK.shaderInfo = shaderInfo;

		std::vector<char> vertexCode = ReadFile((SPIRV_ROOT_SHADER_PATH + shaderInfo.shaderPaths[SHADER_TYPE_VERTEX]).C_Str());
		std::vector<char> fragmentCode = ReadFile((SPIRV_ROOT_SHADER_PATH + shaderInfo.shaderPaths[SHADER_TYPE_FRAGMENT]).C_Str());

		String vertexGLSL = FileReader::ReadFile(String("Res/Shaders/GLSL_450/") + shaderInfo.shaderPaths[SHADER_TYPE_VERTEX]);
		String fragmentGLSL = FileReader::ReadFile(String("Res/Shaders/GLSL_450/") + shaderInfo.shaderPaths[SHADER_TYPE_FRAGMENT]);

		shaderInfoVK.shaderModules[SHADER_TYPE_VERTEX] = CreateShaderModule(vertexCode, device);
		shaderInfoVK.shaderModules[SHADER_TYPE_FRAGMENT] = CreateShaderModule(fragmentCode, device);

		VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info{};
		vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_shader_stage_create_info.module = shaderInfoVK.shaderModules[SHADER_TYPE_VERTEX];
		vertex_shader_stage_create_info.pName = "main";

		VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info{};
		fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragment_shader_stage_create_info.module = shaderInfoVK.shaderModules[SHADER_TYPE_FRAGMENT];
		fragment_shader_stage_create_info.pName = "main";

		shaderInfoVK.shaderStages[SHADER_TYPE_VERTEX] = vertex_shader_stage_create_info;
		shaderInfoVK.shaderStages[SHADER_TYPE_FRAGMENT] = fragment_shader_stage_create_info;

		ParseShader(vertexGLSL, SHADER_TYPE_VERTEX, &shaderInfoVK);
		ParseShader(fragmentGLSL, SHADER_TYPE_FRAGMENT, &shaderInfoVK);

		uint32 shaderID = m_loadedShaders.size();
		m_loadedShaders.push_back(shaderInfoVK);
		return shaderID;
	}

	VkShaderModule RenderContextVK::CreateShaderModule(const std::vector<char>& code, const VkDevice& device)
	{
		VkShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.codeSize = code.size();
		shader_module_create_info.pCode = (uint32_t*)&code[0];

		VkShaderModule shaderModule;
		VkResult err = vkCreateShaderModule(device, &shader_module_create_info, NULL, &shaderModule);
		if (err != VK_SUCCESS)
		{
			std::cerr << "Error creating Vulkan shader module" << std::endl;
		}

		return shaderModule;
	}

	void RenderContextVK::ParseShader(const String& glsl, ShaderType type, ShaderInfoVK * pShaderInfo)
	{
		int32 layoutIndex = glsl.FindFirstOf("layout");
		while (layoutIndex != -1)
		{
			ShaderBufferInfoVK bufferInfo;

			int32 bindingIndex = glsl.FindFirstOf("binding", layoutIndex);
			int32 locationIndex = glsl.FindFirstOf("location", layoutIndex);

			if (bindingIndex == -1)
				break;

			if ((locationIndex != -1) && (locationIndex < bindingIndex))
			{
				layoutIndex = glsl.FindFirstOf("layout", locationIndex);
				continue;
			}

			//find the binding index
			int32 equals = glsl.FindFirstOf("=", bindingIndex);
			int32 rightParenthesis = glsl.FindFirstOf(")", equals);
			String bindingIndexIndex = glsl.SubString(equals + 1, rightParenthesis);
			bindingIndexIndex = bindingIndexIndex.Trim();
			int32 bindingIndexValue = std::stoi(bindingIndexIndex.C_Str());
			bufferInfo.binding = bindingIndexValue;

			//find the ubo name
			int32 uniformIndex = glsl.FindFirstOf("uniform", rightParenthesis);
			int32 sampler2DIndex = glsl.FindFirstOf("sampler2D", uniformIndex);
			int32 semicolonCheckIndex = glsl.FindFirstOf(";", uniformIndex);
			if (sampler2DIndex != -1 && sampler2DIndex < semicolonCheckIndex)
			{
				int32 samplerNameIndex = glsl.FindFirstSpace(sampler2DIndex);
				String samplerName = glsl.SubString(samplerNameIndex, semicolonCheckIndex);
				samplerName = samplerName.Trim();

				SamplerInfoVK samplerInfo;
				samplerInfo.binding = bindingIndexValue;
				samplerInfo.name = samplerName;

				pShaderInfo->samplers.push_back(samplerInfo);
				layoutIndex = glsl.FindFirstOf("layout", uniformIndex);
				continue;
			}

			int32 leftBraceIndex = glsl.FindFirstOf("{", rightParenthesis);

			String uboName = glsl.SubString(uniformIndex + 7, leftBraceIndex);
			uboName = uboName.Trim();
			bufferInfo.name = uboName;

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

			bufferInfo.size = bufferSize;

			bufferInfo.shaderStage = type;

			pShaderInfo->shaderBuffers.push_back(bufferInfo);

			layoutIndex = glsl.FindFirstOf("layout", leftBraceIndex);
		}
	}

	VkPrimitiveTopology RenderContextVK::GetTopology(PrimitiveTopology primitiveTopology)
	{
		switch (primitiveTopology)
		{
		case PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case PRIMITIVE_TOPOLOGY_TRIANGLE_FAN: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		case PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		}
	}

	VkPolygonMode RenderContextVK::GetPolygonMode(PolygonMode polygonMode)
	{
		switch (polygonMode)
		{
		case POLYGON_MODE_FILL: return VK_POLYGON_MODE_FILL;
		case POLYGON_MODE_LINE: return VK_POLYGON_MODE_LINE;
		case POLYGON_MODE_POINT: return VK_POLYGON_MODE_POINT;
		}
	}

	VkCullModeFlagBits RenderContextVK::GetCullMode(CullMode cullMode)
	{
		switch (cullMode)
		{
		case CULL_MODE_FRONT: return VK_CULL_MODE_FRONT_BIT;
		case CULL_MODE_BACK: return VK_CULL_MODE_BACK_BIT;
		case CULL_MODE_NONE: return VK_CULL_MODE_NONE;
		case CULL_MODE_FRONT_BACK: return VK_CULL_MODE_FRONT_AND_BACK;
		}
	}

	VkFrontFace RenderContextVK::GetFrontFace(FrontFace frontFace)
	{
		return frontFace == FRONT_FACE_CW ? VK_FRONT_FACE_CLOCKWISE: VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}

	VkCompareOp RenderContextVK::GetComparison(Comparison comparison)
	{
		switch (comparison)
		{
		case COMPARISON_LESS: return VK_COMPARE_OP_LESS;
		case COMPARISON_GREATER: return VK_COMPARE_OP_GREATER;
		case COMPARISON_LESS_EQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
		case COMPARISON_GREATER_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case COMPARISON_EQUAL: return VK_COMPARE_OP_EQUAL;
		case COMPARISON_NOT_EQUAL: return VK_COMPARE_OP_NOT_EQUAL;
		case COMPARISON_NEVER: return VK_COMPARE_OP_NEVER;
		case COMPARISON_ALWAYS: return VK_COMPARE_OP_ALWAYS;
		}
	}

	VkStencilOp RenderContextVK::GetStencilOperation(StencilOperation stencilOperation)
	{
		switch (stencilOperation)
		{
		case STENCIL_OPERATION_KEEP: return VK_STENCIL_OP_KEEP;
		case STENCIL_OPERATION_ZERO: return VK_STENCIL_OP_ZERO;
		case STENCIL_OPERATION_REPLACE: return VK_STENCIL_OP_REPLACE;
		case STENCIL_OPERATION_INCR_CLAMP: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case STENCIL_OPERATION_DECR_CLAMP: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case STENCIL_OPERATION_INVERT: return VK_STENCIL_OP_INVERT;
		case STENCIL_OPERATION_INCR: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case STENCIL_OPERATION_DECR: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
		}
	}

	VkStencilOpState RenderContextVK::GetStencilFace(const StencilFace & stencilFace)
	{
		VkStencilOpState stencil_op_state {};
		stencil_op_state.compareOp = GetComparison(stencilFace.comparison);
		stencil_op_state.depthFailOp = GetStencilOperation(stencilFace.depthStencilFail);
		stencil_op_state.failOp = GetStencilOperation(stencilFace.stencilFail);
		stencil_op_state.passOp = GetStencilOperation(stencilFace.depthStencilPass);
		stencil_op_state.compareMask = 0xFF;
		stencil_op_state.reference = 0xFF;
		stencil_op_state.writeMask = 0xFF;
		return stencil_op_state;
	}

	VkBlendOp RenderContextVK::GetBlendOperation(BlendOperation blendOperation)
	{
		switch (blendOperation)
		{
		case BLEND_OPERATION_ADD: return VK_BLEND_OP_ADD;
		case BLEND_OPERATION_SUB: return VK_BLEND_OP_SUBTRACT;
		case BLEND_OPERATION_REV_SUB: return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BLEND_OPERATION_MIN: return VK_BLEND_OP_MIN;
		case BLEND_OPERATION_MAX: return VK_BLEND_OP_MAX;
		}
	}

	VkBlendFactor RenderContextVK::GetBlendFactor(BlendFactor blendFactor)
	{
		switch (blendFactor)
		{
		case BLEND_FACTOR_ZERO: return VK_BLEND_FACTOR_ZERO;
		case BLEND_FACTOR_ONE: return VK_BLEND_FACTOR_ONE;
		case BLEND_FACTOR_SRC_COLOR: return VK_BLEND_FACTOR_SRC_COLOR;
		case BLEND_FACTOR_INV_SOURCE_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case BLEND_FACTOR_DEST_COLOR: return VK_BLEND_FACTOR_DST_COLOR;
		case BLEND_FACTOR_INV_DEST_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case BLEND_FACTOR_SRC_ALPHA: return VK_BLEND_FACTOR_SRC_ALPHA;
		case BLEND_FACTOR_SRC_INV_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case BLEND_FACTOR_DEST_ALPHA: return VK_BLEND_FACTOR_DST_ALPHA;
		case BLEND_FACTOR_DEST_INV_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		}
	}

	std::vector<char> RenderContextVK::ReadFile(cstring path)
	{
		FILE* pFile = fopen(path, "rb");

		if (!pFile)
		{
			std::vector<char> error(0);
			std::cerr << "Error finding shader file : " << path << std::endl;
			return error;
		}

		fseek(pFile, 0, SEEK_END);
		long size = ftell(pFile);
		fseek(pFile, 0L, SEEK_SET);

		std::vector<char> loadedFile(size);
		fread(&loadedFile[0], size, 1, pFile);
		fclose(pFile);

		return loadedFile;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) 
	{
		//TODO : Solve this issue
		if (pCallbackData->messageIdNumber == 61)
			return VK_FALSE;

		if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			std::cerr << "Validation Layer: " << pCallbackData->pMessage << "\n\n";
		return VK_FALSE;
	}

	bool window_resize_update_swapchain(WindowEvent windowEvent, const WindowEventInfo & windowEventInfo, void * pUserPtr)
	{
		RenderContextVK* pRenderContext = (RenderContextVK*)pUserPtr;
		pRenderContext->RecreateSwapChain(windowEventInfo.width, windowEventInfo.height);
		return false;
	}

} }