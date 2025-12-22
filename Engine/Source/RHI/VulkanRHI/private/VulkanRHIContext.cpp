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
#include <VulkanOneShotCommandList.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		// NOTE - not constructing staging buffer through VulkanRHIBuffer 
		// because of insufficient number of flags in RHIBuffer::Description
		static void TransferDataToGPU(
			VulkanRHIBuffer::Ptr buffer,
			VmaAllocator allocator,
			VulkanOneShotCommandList& oneShotCmdBuf,
			const RHIBuffer::Description& description) 
		{
			vk::DeviceSize bufferSize = GetBufferSize(description);
			VmaAllocation stagingBufferAllocation = nullptr;

			assert((bufferSize % 4 == 0) && "GPU access must be aligned!");
			assert(description.initData != nullptr);
			VULKAN_RHI_VERIFY(stagingBufferAllocation == nullptr);

			vk::BufferCreateInfo bufInfo = 
			{
				.size = bufferSize,
				.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferSrc,
				.sharingMode = vk::SharingMode::eExclusive
			};

			VmaAllocationCreateInfo allocInfo = 
			{
				.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
				.usage = VMA_MEMORY_USAGE_AUTO,
				.requiredFlags = 0,
				.preferredFlags = 0,
				.memoryTypeBits = 0,
				.pool = nullptr,
				.pUserData = nullptr,
				.priority = 0.0f
			};

			VkBuffer buf;

			VkResult result = vmaCreateBuffer(
				allocator,
				&static_cast<const VkBufferCreateInfo&>(bufInfo),
				&allocInfo,
				&buf,
				&stagingBufferAllocation,
				nullptr);

			VULKAN_RHI_VERIFYF(
				result == VK_SUCCESS,
				"Error {} occured while trying to allocate Staging Buffer",
				vk::to_string(static_cast<vk::Result>(result)));

			VULKAN_RHI_VERIFY(stagingBufferAllocation != nullptr);
			vk::Buffer stagingBuffer = vk::Buffer(buf);

			std::byte* mapped;

			result = vmaMapMemory(allocator, stagingBufferAllocation, reinterpret_cast<void**>(&mapped));
			VULKAN_RHI_VERIFYF(
				result == VK_SUCCESS,
				"Error {} occured while trying to map Staging Buffer memory",
				vk::to_string(static_cast<vk::Result>(result)));

			std::memcpy(mapped, description.initData, bufferSize);

			vmaUnmapMemory(allocator, stagingBufferAllocation);

			vk::CommandBuffer commandBuffer = oneShotCmdBuf.Start();

			VULKAN_RHI_CHECK_RESULT(commandBuffer.begin(vk::CommandBufferBeginInfo{}));
			{
				vk::BufferCopy2 copy = 
				{
					.srcOffset = 0,
					.dstOffset = 0,
					.size = bufferSize
				};

				vk::CopyBufferInfo2 copyInfo = 
				{
					.srcBuffer = stagingBuffer,
					.dstBuffer = buffer->GetBuffer(),
					.regionCount = 1,
					.pRegions = &copy
				};

				commandBuffer.copyBuffer2(copyInfo);
			}
			VULKAN_RHI_CHECK_RESULT(commandBuffer.end());

			oneShotCmdBuf.SubmitAndWait(std::move(commandBuffer));

			VULKAN_RHI_VERIFY(stagingBufferAllocation != nullptr);

			vmaDestroyBuffer(allocator, VkBuffer(stagingBuffer), stagingBufferAllocation);
		}

		VulkanRHIContext::VulkanRHIContext()
		{
			m_WorkCounter = std::make_unique<VulkanWorkCounter>();

			m_Instance = new VulkanRHIFactory();
			m_Device = new VulkanRHIDevice(m_Instance);
			m_MemoryAllocator = new VulkanMemoryAllocator(m_Instance, m_Device);

			m_Fence = new VulkanRHIFence(*m_WorkCounter, m_Device);
			m_CommandQueue = new VulkanRHICommandQueue(m_Device);
			m_DescriptorPool = new VulkanDescriptorPool(*m_WorkCounter, m_Device);

			m_SwapChain = new VulkanRHISwapChain(*m_WorkCounter, m_Instance, m_Device, m_CommandQueue, m_Fence);

			m_CommandBuffer = new VulkanRHICommandList(*m_WorkCounter, m_Device, *m_DescriptorPool, m_SwapChain.Get());

			m_HLSLCompiler = std::make_unique<VulkanHLSLCompiler>();

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
				TransferDataToGPU(buffer, m_MemoryAllocator->GetAllocator(), *m_OneShotCommandList, description);
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
				TransferDataToGPU(buffer, m_MemoryAllocator->GetAllocator(), *m_OneShotCommandList, description);
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
			
			// NOTE - it is obviously bad to do it like this, one buffer at a time, but for now I think it's fine
			if (description.UsageFlag == RHIBuffer::UsageFlag::GpuReadOnly) 
			{
				TransferDataToGPU(buffer, m_MemoryAllocator->GetAllocator(), *m_OneShotCommandList, description);
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
					ASSERT_NOT_IMPLEMENTED;
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
			VULKAN_RHI_VERIFY(description.NumRenderTargets <= 8);

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
			assert(vertexDesc.initData);
			assert(indexDesc.initData);

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