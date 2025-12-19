#include "VulkanRHICore.h"

#include "Geometry.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		static uint32_t sizeFromFormat(vk::Format format)
		{
			switch (format) {
			case vk::Format::eR32G32B32Sfloat:
				return sizeof(float) * 3;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return 0;
			}
		}

		vk::Format ConvertToVkFormat(const ResourceFormat& resourceFormat)
		{
			switch (resourceFormat) {
			case ResourceFormat::RGBA8_UNORM:
				return vk::Format::eR8G8B8A8Unorm;
			case ResourceFormat::RGB32_FLOAT:
				return vk::Format::eR32G32B32Sfloat;
			case ResourceFormat::D24S8:
				return vk::Format::eD24UnormS8Uint;
			case ResourceFormat::R16_UNORM:
				return vk::Format::eR16Unorm;
			case ResourceFormat::R16_UINT:
				return vk::Format::eR16Uint;
			case ResourceFormat::BGRA8_UNORM:
				return vk::Format::eB8G8R8A8Unorm;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::Format::eUndefined;
			}
		}

		ResourceFormat ConvertToResourceFormat(vk::Format resourceFormat)
		{
			switch (resourceFormat) {
			default:
				ASSERT_NOT_IMPLEMENTED;
				return ResourceFormat::UNKNOWN;
			}
		}

		vk::IndexType ConvertToVkIndexType(const ResourceFormat& resourceFormat)
		{
			switch (resourceFormat)
			{
			case ResourceFormat::R16_UINT:
				return vk::IndexType::eUint16;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::IndexType::eUint16;
			}
		}

		std::string GetShaderTarget(const RHITechnique::ShaderInfoDescription::ShaderType& shaderType)
		{
			switch (shaderType)
			{
			case RHITechnique::ShaderInfoDescription::ShaderType::VertexShader:
				return "vs_5_0";
			case RHITechnique::ShaderInfoDescription::ShaderType::PixelShader:
				return "ps_5_0";
			default:
				ASSERT_NOT_IMPLEMENTED;
				return "undefined";
			}
		}

		vk::ShaderStageFlagBits ConvertToShaderStage(const RHITechnique::ShaderInfoDescription::ShaderType& shaderType)
		{
			switch (shaderType)
			{
			case RHITechnique::ShaderInfoDescription::ShaderType::VertexShader:
				return vk::ShaderStageFlagBits::eVertex;
			case RHITechnique::ShaderInfoDescription::ShaderType::PixelShader:
				return vk::ShaderStageFlagBits::eFragment;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::ShaderStageFlagBits(0);
			}
		}

		vk::VertexInputRate ConvertToVkInputRate(const RHITechnique::InputLayoutDescription::Classification& classification)
		{
			switch (classification)
			{
			case RHITechnique::InputLayoutDescription::Classification::PerVertex:
				return vk::VertexInputRate::eVertex;
			case RHITechnique::InputLayoutDescription::Classification::PerInstance:
				return vk::VertexInputRate::eInstance;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::VertexInputRate::eVertex;
			}
		}

		vk::VertexInputBindingDescription GetBindingDescriptions(const RHITechnique::InputLayout& inputLayout)
		{
			assert(!inputLayout.empty());

			// NOTE - assuming one vertex type
			return vk::VertexInputBindingDescription{
						.binding = inputLayout[0].InputSlot,
						.stride = sizeof(RenderCore::Geometry::VertexType),
						.inputRate = ConvertToVkInputRate(inputLayout[0].InputSlotClass)
			};
		}

		std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions(const RHITechnique::InputLayout& inputLayout)
		{
			std::vector<vk::VertexInputAttributeDescription> description;
			description.reserve(inputLayout.size());

			uint32_t offset = 0;
			for (const RHITechnique::InputLayoutDescription& desc : inputLayout)
			{
				vk::Format format = ConvertToVkFormat(desc.Format);
				description.emplace_back(vk::VertexInputAttributeDescription{
					.location = desc.Index,
					.binding = desc.InputSlot,
					.format = format,
					.offset = offset
					});

				offset += sizeFromFormat(format);
			}

			return description;
		}

		vk::PrimitiveTopology ConvertToVkPrimitiveTopology(const PrimitiveTopologyType& primitiveTopologyType)
		{
			switch (primitiveTopologyType)
			{
			case PrimitiveTopologyType::Triangle:
				return vk::PrimitiveTopology::eTriangleList;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::PrimitiveTopology::eTriangleList;
			}
		}

		vk::PipelineRasterizationStateCreateInfo ConvertToVkRasterizerInfo(const RasterizerDescription& rasterState)
		{
			return vk::PipelineRasterizationStateCreateInfo{
				.depthClampEnable = static_cast<vk::Bool32>(rasterState.DepthClipEnable),
				.rasterizerDiscardEnable = vk::False,
				.polygonMode =
					(rasterState.fillMode == FillMode::Solid)
					? vk::PolygonMode::eFill
					: vk::PolygonMode::eLine,
				.cullMode =
					(rasterState.cullMode == CullMode::Back)
					? vk::CullModeFlagBits::eBack
					: (rasterState.cullMode == CullMode::Front)
						? vk::CullModeFlagBits::eFront
						: vk::CullModeFlagBits::eNone,
				.frontFace =
					(rasterState.FrontCounterClockwise)
					? vk::FrontFace::eCounterClockwise
					: vk::FrontFace::eClockwise,
				.depthBiasEnable = static_cast<vk::Bool32>(rasterState.DepthBias),
				.depthBiasClamp = rasterState.DepthBiasClamp,
				.depthBiasSlopeFactor = rasterState.SlopeScaledDepthBias,
				.lineWidth = 1.0f
			};
		}

		vk::PipelineMultisampleStateCreateInfo GetMultisampleInfo(
			const RasterizerDescription& rasterState, const BlendDescription& blendDesc)
		{
			return vk::PipelineMultisampleStateCreateInfo{
				.rasterizationSamples =
					(rasterState.MultisampleEnable)
					? vk::SampleCountFlagBits::e4
					: vk::SampleCountFlagBits::e1,
				.sampleShadingEnable = static_cast<vk::Bool32>(rasterState.MultisampleEnable),
				.alphaToCoverageEnable = static_cast<vk::Bool32>(blendDesc.AlphaToCoverageEnable)
			};
		}

		vk::CompareOp ConvertToVkCompareOp(const ComparisonFunc& compFunc)
		{
			switch (compFunc)
			{
			case ComparisonFunc::Less:
				return vk::CompareOp::eLess;
			case ComparisonFunc::Always:
				return vk::CompareOp::eAlways;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::CompareOp::eLess;
			}
		}

		vk::StencilOp ConvertToVkStencilOp(const StencilOp& stencilOp)
		{
			switch (stencilOp)
			{
			case StencilOp::Keep:
				return vk::StencilOp::eKeep;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::StencilOp::eKeep;
			}
		}

		vk::StencilOpState ConvertToVkStencilOpState(
			const StencilOpDescription& stencilOpDescription, uint32_t compareMask, uint32_t writeMask, uint32_t referenceMask)
		{
			return vk::StencilOpState{
				.failOp = ConvertToVkStencilOp(stencilOpDescription.StencilFailOp),
				.passOp = ConvertToVkStencilOp(stencilOpDescription.StencilPassOp),
				.depthFailOp = ConvertToVkStencilOp(stencilOpDescription.StencilDepthFailOp),
				.compareOp = ConvertToVkCompareOp(stencilOpDescription.StencilFunc),
				.compareMask = compareMask,
				.writeMask = writeMask,
				.reference = referenceMask
			};
		}

		vk::PipelineDepthStencilStateCreateInfo ConvertToVkDepthStencilInfo(const DepthStencilDescription& description)
		{
			return vk::PipelineDepthStencilStateCreateInfo{
				.depthTestEnable = static_cast<vk::Bool32>(description.DepthEnable),
				.depthWriteEnable = static_cast<vk::Bool32>(description.DepthEnable),
				.depthCompareOp = ConvertToVkCompareOp(description.DepthFunc),
				.stencilTestEnable = static_cast<vk::Bool32>(description.StencilEnable),
				.front = ConvertToVkStencilOpState(
					description.FrontFace,
					static_cast<uint32_t>(description.StencilReadMask),
					static_cast<uint32_t>(description.StencilWriteMask),
					static_cast<uint32_t>(description.StencilReadMask)),
				.back = ConvertToVkStencilOpState(
					description.BackFace,
					static_cast<uint32_t>(description.StencilReadMask),
					static_cast<uint32_t>(description.StencilWriteMask),
					static_cast<uint32_t>(description.StencilReadMask)),
				.maxDepthBounds = 1.0f
			};
		}

		vk::BlendFactor ConvertToVkBlendFactor(const Blend& blend)
		{
			switch (blend)
			{
			case Blend::One:
				return vk::BlendFactor::eOne;
			case Blend::Zero:
				return vk::BlendFactor::eZero;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::BlendFactor::eOne;
			}
		}

		vk::BlendOp ConvertToVkBlendOp(const BlendOperation& blendOp)
		{
			switch (blendOp)
			{
			case BlendOperation::Add:
				return vk::BlendOp::eAdd;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::BlendOp::eAdd;
			}
		}

		vk::LogicOp ConvertToVkLogicOp(const LogicOperation& logicOp)
		{
			switch (logicOp)
			{
			case LogicOperation::Noop:
				return vk::LogicOp::eNoOp;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::LogicOp::eNoOp;
			}
		}

		vk::PipelineColorBlendAttachmentState ConvertToVkColorBlendAttachment(const RenderTargetBlendDescription& description)
		{
			vk::ColorComponentFlags mask{};

			if (description.RenderTargetWriteMask & 1u)
			{
				mask = mask | vk::ColorComponentFlagBits::eR;
			}
			if (description.RenderTargetWriteMask & 2u)
			{
				mask = mask | vk::ColorComponentFlagBits::eG;
			}
			if (description.RenderTargetWriteMask & 4u)
			{
				mask = mask | vk::ColorComponentFlagBits::eB;
			}
			if (description.RenderTargetWriteMask & 8u)
			{
				mask = mask | vk::ColorComponentFlagBits::eA;
			}

			return vk::PipelineColorBlendAttachmentState{
				.blendEnable = static_cast<vk::Bool32>(description.BlendEnable),
				.srcColorBlendFactor = ConvertToVkBlendFactor(description.SrcBlend),
				.dstColorBlendFactor = ConvertToVkBlendFactor(description.DestBlend),
				.colorBlendOp = ConvertToVkBlendOp(description.BlendOp),
				.srcAlphaBlendFactor = ConvertToVkBlendFactor(description.SrcBlendAlpha),
				.dstAlphaBlendFactor = ConvertToVkBlendFactor(description.DestBlendAlpha),
				.alphaBlendOp = ConvertToVkBlendOp(description.BlendOpAlpha),
				.colorWriteMask = mask
			};
		}

		vk::ImageType GetImageType(const RHITexture::Dimensions& dimension)
		{
			switch (dimension)
			{
			case RHITexture::Dimensions::One:
				return vk::ImageType::e1D;
			case RHITexture::Dimensions::Two:
				return vk::ImageType::e2D;
			case RHITexture::Dimensions::Three:
				return vk::ImageType::e3D;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::ImageType::e1D;
			}
		}

		vk::ImageViewType GetImageViewType(const RHITexture::Dimensions& dimension)
		{
			switch (dimension)
			{
			case RHITexture::Dimensions::One:
				return vk::ImageViewType::e1D;
			case RHITexture::Dimensions::Two:
				return vk::ImageViewType::e2D;
			case RHITexture::Dimensions::Three:
				return vk::ImageViewType::e3D;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::ImageViewType::e1D;
			}
		}

		vk::Extent3D GetImageExtent(const RHITexture::Description& description)
		{
			return vk::Extent3D{
				.width = static_cast<uint32_t>(description.Width),
				.height = static_cast<uint32_t>(description.Height),
				.depth = 1 };
		}

		vk::ImageUsageFlags GetImageUsageFlags(const RHITexture::UsageFlags::Flag& flags)
		{
			vk::ImageUsageFlags usageFlags{};

			if (flags & RHITexture::UsageFlags::ShaderResource)
			{
				usageFlags = usageFlags | vk::ImageUsageFlagBits::eSampled;
			}

			if (flags & RHITexture::UsageFlags::RenderTarget)
			{
				usageFlags = usageFlags | vk::ImageUsageFlagBits::eColorAttachment;
			}

			if (flags & RHITexture::UsageFlags::DepthStencil)
			{
				usageFlags = usageFlags | vk::ImageUsageFlagBits::eDepthStencilAttachment;
			}

			return usageFlags;
		}

		vk::ImageAspectFlags GetImageAspectFlags(vk::Format format)
		{
			switch (format)
			{
			case vk::Format::eS8Uint:
				return vk::ImageAspectFlagBits::eStencil;
			case vk::Format::eD16Unorm:
			case vk::Format::eD32Sfloat:
				return vk::ImageAspectFlagBits::eDepth;
			case vk::Format::eD16UnormS8Uint:
			case vk::Format::eD24UnormS8Uint:
			case vk::Format::eD32SfloatS8Uint:
				return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
			default:
				return vk::ImageAspectFlagBits::eColor;
			}
		}

		vk::DeviceSize GetBufferSize(const RHIBuffer::Description& description)
		{
			return static_cast<vk::DeviceSize>(description.Count * description.ElementSize);
		}

		vk::BufferUsageFlags GetBufferUsageFlags(const RHIBuffer::UsageFlag& flag)
		{
			switch (flag)
			{
			case RHIBuffer::UsageFlag::ConstantBuffer:
				return vk::BufferUsageFlagBits::eUniformBuffer;
			case RHIBuffer::UsageFlag::CpuWrite:
				return vk::BufferUsageFlagBits::eUniformBuffer;
			case RHIBuffer::UsageFlag::GpuReadOnly:
				return vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst;
			default:
				ASSERT_NOT_IMPLEMENTED;
				return vk::BufferUsageFlagBits::eUniformBuffer;
			}
		}
	}
}