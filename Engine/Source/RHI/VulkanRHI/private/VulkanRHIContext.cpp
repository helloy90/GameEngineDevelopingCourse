#include <VulkanRHIContext.h>

#include <FileSystem.h>
#include <RenderCore.h>
#include <RHICommon.h>

#include <Vulkan.h>

#include <VulkanRHICore.h>
#include <VulkanUtil.h>

#include <VulkanWorkCounter.h>

#include <VulkanRHIFactory.h>
#include <VulkanRHIDevice.h>
#include <VulkanRHICommandQueue.h>
#include <VulkanRHISwapchain.h>

#include <VulkanRHICommandList.h>
#include <VulkanRHIFence.h>

#include <VulkanRHITechnique.h>
#include <VulkanRHIPipelineStateObject.h>

#include <VulkanRHITexture.h>
#include <VulkanRHIBuffer.h>

#include <VulkanDescriptorPool.h>
#include <VulkanHLSLCompiler.h>
#include <VulkanMemoryAllocator.h>
#include <VulkanTransferHelper.h>
#include <VulkanOneShotCommandList.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		static vk::DeviceSize s_stagingBufferSize = 1 << 16; // 65536 bytes

		VulkanRHIContext::VulkanRHIContext()
		{
			m_WorkCounter = std::make_unique<VulkanWorkCounter>();

			m_Instance = new VulkanRHIFactory();
			m_Device = new VulkanRHIDevice(m_Instance);
			m_MemoryAllocator = new VulkanMemoryAllocator(m_Instance, m_Device);

			m_Fence = new VulkanRHIFence(*m_WorkCounter, m_Device);
			m_CommandQueue = new VulkanRHICommandQueue(m_Device, m_Fence.Get());
			m_DescriptorPool = new VulkanDescriptorPool(*m_WorkCounter, m_Device);

			m_SwapChain = new VulkanRHISwapChain(*m_WorkCounter, m_Instance, m_Device, m_CommandQueue, m_Fence);

			m_CommandBuffer = new VulkanRHICommandList(*m_WorkCounter, m_Device, *m_DescriptorPool, m_SwapChain.Get());

			m_HLSLCompiler = std::make_unique<VulkanHLSLCompiler>();
			m_TransferHelper = new VulkanTransferHelper(m_MemoryAllocator->GetAllocator(), s_stagingBufferSize);

			m_CommandBuffer->Reset();

			m_OneShotCommandList = std::make_unique<VulkanOneShotCommandList>(m_Device, m_CommandQueue);
		}

		VulkanRHIContext::~VulkanRHIContext()
		{
			VULKAN_RHI_CHECK_RESULT(m_Device->GetDevice().waitIdle());
		}

		RHIBuffer::Ptr VulkanRHIContext::CreateVertexBuffer(RHIBuffer::Description&& description)
		{
			VulkanRHIBuffer::Ptr buffer = new VulkanRHIBuffer(
				description,
				m_MemoryAllocator->GetAllocator(),
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);

			if (description.UsageFlag == RHIBuffer::UsageFlag::GpuReadOnly) 
			{
				vk::DeviceSize bufferSize = GetBufferSize(description);

				ENGINE_ASSERTF(bufferSize < s_stagingBufferSize, "Staging buffer is to small for this buffer!");

				vk::CommandBuffer cmdBuf = m_OneShotCommandList->Start();

				VULKAN_RHI_CHECK_RESULT(cmdBuf.begin(vk::CommandBufferBeginInfo{}));
				{
					m_TransferHelper->UploadBuffer(
						buffer,
						cmdBuf,
						bufferSize,
						description.initData
					);
				}
				VULKAN_RHI_CHECK_RESULT(cmdBuf.end());

				m_OneShotCommandList->SubmitAndWait(cmdBuf);
			}

			return buffer;
		}

		RHIBuffer::Ptr VulkanRHIContext::CreateIndexBuffer(RHIBuffer::Description&& description)
		{
			VulkanRHIBuffer::Ptr buffer = new VulkanRHIBuffer(
				description,
				m_MemoryAllocator->GetAllocator(),
				vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);

			if (description.UsageFlag == RHIBuffer::UsageFlag::GpuReadOnly) 
			{
				vk::DeviceSize bufferSize = GetBufferSize(description);

				ENGINE_ASSERTF(bufferSize < s_stagingBufferSize, "Staging buffer is to small for this buffer!");

				vk::CommandBuffer cmdBuf = m_OneShotCommandList->Start();

				VULKAN_RHI_CHECK_RESULT(cmdBuf.begin(vk::CommandBufferBeginInfo{}));
				{
					m_TransferHelper->UploadBuffer(
						buffer,
						cmdBuf,
						bufferSize,
						description.initData
					);
				}
				VULKAN_RHI_CHECK_RESULT(cmdBuf.end());

				m_OneShotCommandList->SubmitAndWait(cmdBuf);
			}

			return buffer;
		}

		RHITexture::Ptr VulkanRHIContext::CreateTexture(const RHITexture::Description& description)
		{
			// NOTE - doing all the work in texture constuctor, because of memory allocations
			// (so destructor can handle freeing it)
			return VulkanRHITexture::Ptr(
				new VulkanRHITexture(description, m_Device, m_MemoryAllocator->GetAllocator())
			);
		}

		RHIBuffer::Ptr VulkanRHIContext::CreateBuffer(RHIBuffer::Description&& description)
		{
			// NOTE - doing almost all the work in buffer constuctor, because of memory allocations
			// (so destructor can handle freeing it)
			VulkanRHIBuffer::Ptr buffer = new VulkanRHIBuffer(description, m_MemoryAllocator->GetAllocator());
			
			if (description.UsageFlag == RHIBuffer::UsageFlag::GpuReadOnly) 
			{
				vk::DeviceSize bufferSize = GetBufferSize(description);

				ENGINE_ASSERTF(bufferSize < s_stagingBufferSize, "Staging buffer is to small for this buffer!");

				m_TransferHelper->UploadBuffer(
					buffer,
					m_CommandBuffer->GetCurrentBuffer(),
					bufferSize,
					description.initData
				);
			}

			return buffer;
		}

		RHITechnique::Ptr VulkanRHIContext::CreateTechnique(
			const RHITechnique::ShaderInfo& shaderInfo,
			const RHITechnique::InputLayout& inputLayout,
			const RHITechnique::RootSignature& rootSignature)
		{
			// NOTE - there is only one space (or set in vulkan terms), so using one bindings array
			std::vector<vk::DescriptorSetLayoutBinding> bindings;
			bindings.reserve(rootSignature.size());

			for (const RHITechnique::RootSignatureDescription& signatureDesc : rootSignature)
			{
				if (signatureDesc.IsConstantBuffer) 
				{
					bindings.emplace_back(
						vk::DescriptorSetLayoutBinding
						{
							.binding = signatureDesc.SlotIndex,
							.descriptorType = vk::DescriptorType::eUniformBuffer,
							.descriptorCount = 1,
							// NOTE - there is no way right now to find out from root signature 
							// for what stage of shader program is buffer used, so assuming both shader stages
							.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
							.pImmutableSamplers = nullptr
						});
				}
				else 
				{
					ENGINE_ASSERT_NOT_IMPLEMENTED;
				}
			}

			vk::DescriptorSetLayoutCreateInfo descSetLayoutCreateInfo =
			{
				.bindingCount = static_cast<uint32_t>(bindings.size()),
				.pBindings = bindings.data()
			};

			vk::UniqueDescriptorSetLayout descriptorSetLayout = VulkanUtil::GetCheckedVkValue(m_Device->GetDevice().createDescriptorSetLayoutUnique(descSetLayoutCreateInfo));

			VulkanRHITechnique::ShaderStagesList shaderStagesList;
			shaderStagesList.reserve(shaderInfo.size());

			for (const RHITechnique::ShaderInfoDescription& shaderDesc : shaderInfo)
			{
				std::wstring shaderPath = Core::g_FileSystem->GetShaderPath(shaderDesc.ShaderFile);

				RefCountPtr<IDxcBlob> code = m_HLSLCompiler->CompileShader(shaderPath, shaderDesc.EntryPoint, GetShaderTarget(shaderDesc.Type));

				vk::ShaderModuleCreateInfo createInfo = 
				{
					.codeSize = code->GetBufferSize(),
					.pCode = reinterpret_cast<const uint32_t*>(code->GetBufferPointer())
				};

				vk::UniqueShaderModule shaderModule = VulkanUtil::GetCheckedVkValue(m_Device->GetDevice().createShaderModuleUnique(createInfo));

				shaderStagesList.emplace_back(std::move(shaderModule));
			}

			vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = 
			{
				.setLayoutCount = 1,
				.pSetLayouts = &descriptorSetLayout.get(),
				.pushConstantRangeCount = 0
			};

			vk::UniquePipelineLayout pipelineLayout = VulkanUtil::GetCheckedVkValue(m_Device->GetDevice().createPipelineLayoutUnique(pipelineLayoutCreateInfo));

			return VulkanRHITechnique::Ptr(
				new VulkanRHITechnique(
					shaderInfo,
					inputLayout,
					rootSignature,
					*m_WorkCounter,
					std::move(pipelineLayout),
					std::move(descriptorSetLayout),
					std::move(shaderStagesList)));
		}

		RHIPipelineStateObject::Ptr VulkanRHIContext::CreatePSO(const RHIPipelineStateObject::Description& description)
		{
			// NOTE - checking because of hardcoded render target array size in description
			ENGINE_ASSERT(description.NumRenderTargets <= 8);

			VulkanRHITechnique* vkTechnique = reinterpret_cast<VulkanRHITechnique*>(description.Technique.Get());
			const RHITechnique::InputLayout& generalLayout = vkTechnique->GetGeneralInputLayout();

			vk::VertexInputBindingDescription bindingDesc = GetBindingDescriptions(generalLayout);
			std::vector<vk::VertexInputAttributeDescription> attributeDesc = GetAttributeDescriptions(generalLayout);

			vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.setVertexBindingDescriptions({ bindingDesc });
			vertexInputInfo.setVertexAttributeDescriptions(attributeDesc);

			vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = 
			{
				.topology = ConvertToVkPrimitiveTopology(description.PrimitiveTopology)
			};

			vk::PipelineViewportStateCreateInfo viewportStateInfo = 
			{
				.viewportCount = 1,
				.scissorCount = 1
			};

			vk::PipelineRasterizationStateCreateInfo rasteriserInfo = ConvertToVkRasterizerInfo(description.RasterState);

			vk::PipelineMultisampleStateCreateInfo multisamplingInfo = GetMultisampleInfo(description.RasterState, description.BlendState);

			vk::PipelineDepthStencilStateCreateInfo depthStencilInfo = ConvertToVkDepthStencilInfo(description.DepthStencilState);

			std::vector<vk::PipelineColorBlendAttachmentState> attachments;
			attachments.reserve(description.NumRenderTargets);
			// NOTE - IndependentBlendEnable is unused as it is not a parameter of a pipeline,
			// but a vulkan feature, that should be turned on when creating vulkan device.
			// If this feature is turned on, then no additional configuring is required for it to work.
			for (uint32_t i = 0; i < description.NumRenderTargets; i++)
			{
				attachments.emplace_back(ConvertToVkColorBlendAttachment(description.BlendState.RenderTarget[i]));
			}

			vk::PipelineColorBlendStateCreateInfo colorBlendingInfo = 
			{
				// NOTE - assuming that all logic ops are equal in render targets, because they should be in vulkan
				.logicOpEnable =
					(description.NumRenderTargets > 0)
					? static_cast<vk::Bool32>(description.BlendState.RenderTarget[0].LogicOpEnable)
					: vk::False,
				.logicOp =
					(description.NumRenderTargets > 0)
					? ConvertToVkLogicOp(description.BlendState.RenderTarget[0].LogicOp)
					: vk::LogicOp::eNoOp
			};
			colorBlendingInfo.setAttachments(attachments);

			std::vector dynamicStates = 
			{
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor
			};
			vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
			dynamicStateInfo.setDynamicStates(dynamicStates);

			std::vector<vk::Format> colorAttachmentFormats{};
			colorAttachmentFormats.reserve(description.NumRenderTargets);
			for (uint32_t i = 0; i < description.NumRenderTargets; i++)
			{
				colorAttachmentFormats.emplace_back(ConvertToVkFormat(description.RTVFormats[i]));
			}

			vk::PipelineRenderingCreateInfo renderingInfo = 
			{
				.depthAttachmentFormat = ConvertToVkFormat(description.DSVFormat),
				.stencilAttachmentFormat = ConvertToVkFormat(description.DSVFormat)
			};
			renderingInfo.setColorAttachmentFormats(colorAttachmentFormats);

			vk::GraphicsPipelineCreateInfo pipelineInfo = 
			{
				.pNext = &renderingInfo,
				.pVertexInputState = &vertexInputInfo,
				.pInputAssemblyState = &inputAssemblyInfo,
				.pViewportState = &viewportStateInfo,
				.pRasterizationState = &rasteriserInfo,
				.pMultisampleState = &multisamplingInfo,
				.pDepthStencilState = &depthStencilInfo,
				.pColorBlendState = &colorBlendingInfo,
				.pDynamicState = &dynamicStateInfo,
				.layout = vkTechnique->GetPipelineLayout()
			};

			std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

			const RHITechnique::ShaderInfo& shaderInfo = vkTechnique->GetGeneralShaderInfo();
			const VulkanRHITechnique::ShaderStagesList& shaderModules = vkTechnique->GetShaderStages();

			shaderStages.reserve(shaderModules.size());
			for (std::size_t i = 0; i < shaderModules.size(); i++) 
			{
				shaderStages.emplace_back(vk::PipelineShaderStageCreateInfo
					{
						.stage = ConvertToShaderStage(shaderInfo[i].Type),
						.module = shaderModules[i].get(),
						.pName = shaderInfo[i].EntryPoint.c_str()
					});
			}
			pipelineInfo.setStages(shaderStages);

			vk::UniquePipeline graphicsPipeline = VulkanUtil::GetCheckedVkValue(m_Device->GetDevice().createGraphicsPipelineUnique(nullptr, pipelineInfo));

			return VulkanRHIPipelineStateObject::Ptr(
				new VulkanRHIPipelineStateObject(description, std::move(graphicsPipeline))
			);
		}

		RHIMesh::Ptr VulkanRHIContext::CreateMesh(
			const RHIMesh::VertexBufferDescription& vertexDesc,
			const RHIMesh::IndexBufferDescription& indexDesc)
		{
			ENGINE_ASSERT(vertexDesc.initData);
			ENGINE_ASSERT(indexDesc.initData);

			RHIBuffer::Ptr vertexBuffer = CreateVertexBuffer(
				{
					.Count = vertexDesc.Count,
					.ElementSize = vertexDesc.ElementSize,
					.UsageFlag = HAL::RHIBuffer::UsageFlag::GpuReadOnly,
					.initData = vertexDesc.initData
				}
			);

			RHIBuffer::Ptr indexBuffer = CreateIndexBuffer(
				{
					.Count = indexDesc.Count,
					.ElementSize = GetFormatSize(indexDesc.Format),
					.UsageFlag = HAL::RHIBuffer::UsageFlag::GpuReadOnly,
					.initData = indexDesc.initData
				}
			);

			return RHIMesh::Ptr(new RHIMesh(vertexBuffer, indexBuffer, indexDesc.Format));
		}

		void VulkanRHIContext::SetDescriptorHeaps()
		{
			m_DescriptorPool->ResetCurrent();
		}

		RHIDevice::Ptr VulkanRHIContext::GetDevice() const
		{
			return m_Device;
		}

		RHIFactory::Ptr VulkanRHIContext::GetFactory() const
		{
			return m_Instance;
		}

		RHISwapChain::Ptr VulkanRHIContext::GetSwapChain() const
		{
			return m_SwapChain;
		}

		RHIFence::Ptr VulkanRHIContext::GetFence() const
		{
			return m_Fence;
		}

		RHICommandQueue::Ptr VulkanRHIContext::GetCommandQueue() const
		{
			return m_CommandQueue;
		}

		RHICommandList::Ptr VulkanRHIContext::GetCommandList() const
		{
			return m_CommandBuffer;
		}

		vk::PhysicalDevice VulkanRHIContext::GetPhysicalDevice() const 
		{
			return m_Device->GetPhysicalDevice();
		}

		uint32_t VulkanRHIContext::GetQueueIdx() const 
		{
			return m_Device->GetUniversalQueueIdx();
		}
	}
}