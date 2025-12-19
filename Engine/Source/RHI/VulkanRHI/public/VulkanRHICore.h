#pragma once

#include "RHI/VulkanRHI/export.h"

#include "RHICore.h"
#include "RHITechnique.h"
#include "RHITexture.h"
#include "RHIBuffer.h"

#include "vulkan/vulkan.hpp"

namespace GameEngine
{
	namespace Render::HAL
	{
		VULKAN_API vk::Format ConvertToVkFormat(const ResourceFormat& resourceFormat);
		VULKAN_API ResourceFormat ConvertToResourceFormat(vk::Format resourceFormat);
		VULKAN_API vk::IndexType ConvertToVkIndexType(const ResourceFormat& resourceFormat);

		VULKAN_API std::string GetShaderTarget(const RHITechnique::ShaderInfoDescription::ShaderType& shaderType);
		VULKAN_API vk::ShaderStageFlagBits ConvertToShaderStage(const RHITechnique::ShaderInfoDescription::ShaderType& shaderType);
		VULKAN_API vk::VertexInputRate ConvertToVkInputRate(const RHITechnique::InputLayoutDescription::Classification& classification);
		VULKAN_API vk::VertexInputBindingDescription GetBindingDescriptions(const RHITechnique::InputLayout& inputLayout);
		VULKAN_API std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions(const RHITechnique::InputLayout& inputLayout);
		VULKAN_API vk::PrimitiveTopology ConvertToVkPrimitiveTopology(const PrimitiveTopologyType& primitiveTopologyType);
		VULKAN_API vk::PipelineRasterizationStateCreateInfo ConvertToVkRasterizerInfo(const RasterizerDescription& rasterState);
		VULKAN_API vk::PipelineMultisampleStateCreateInfo GetMultisampleInfo(
			const RasterizerDescription& rasterState, const BlendDescription& blendDesc);
	
		VULKAN_API vk::CompareOp ConvertToVkCompareOp(const ComparisonFunc& compFunc);
		VULKAN_API vk::StencilOp ConvertToVkStencilOp(const StencilOp& stencilOp);
		VULKAN_API vk::StencilOpState ConvertToVkStencilOpState(
			const StencilOpDescription& stencilOpDescription, uint32_t compareMask, uint32_t writeMask, uint32_t referenceMask);
		VULKAN_API vk::PipelineDepthStencilStateCreateInfo ConvertToVkDepthStencilInfo(const DepthStencilDescription& description);

		VULKAN_API vk::BlendFactor ConvertToVkBlendFactor(const Blend& blend);
		VULKAN_API vk::BlendOp ConvertToVkBlendOp(const BlendOperation& blendOp);
		VULKAN_API vk::LogicOp ConvertToVkLogicOp(const LogicOperation& logicOp);
		VULKAN_API vk::PipelineColorBlendAttachmentState ConvertToVkColorBlendAttachment(const RenderTargetBlendDescription& description);

		VULKAN_API vk::ImageType GetImageType(const RHITexture::Dimensions& dimension);
		VULKAN_API vk::ImageViewType GetImageViewType(const RHITexture::Dimensions& dimension);
		VULKAN_API vk::Extent3D GetImageExtent(const RHITexture::Description& description);
		VULKAN_API vk::ImageUsageFlags GetImageUsageFlags(const RHITexture::UsageFlags::Flag& flags);
		VULKAN_API vk::ImageAspectFlags GetImageAspectFlags(vk::Format format);
		VULKAN_API vk::DeviceSize GetBufferSize(const RHIBuffer::Description& description);
		VULKAN_API vk::BufferUsageFlags GetBufferUsageFlags(const RHIBuffer::UsageFlag& flag);
		
	}
}