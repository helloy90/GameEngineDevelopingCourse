#include <VulkanRHICommandList.h>

#include <RenderCore.h>

#include <VulkanUtil.h>

#include <VulkanRHICore.h>

#include <VulkanRHIBuffer.h>
#include <VulkanRHIPipelineStateObject.h>
#include <VulkanRHISwapChain.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanRHICommandList::VulkanRHICommandList(
			const VulkanWorkCounter& workCounter,
			VulkanRHIDevice::Ptr device,
			VulkanDescriptorPool& pool,
			VulkanRHISwapChain* swapchain
		) : m_WorkCounter(workCounter)
			, m_Pool(pool)
			, m_Device(device->GetDevice())
			, m_SwapChain(swapchain)
		{
			m_CommandAllocator = new VulkanRHICommandAllocator(device);

			vk::CommandBufferAllocateInfo info = 
			{
				.commandPool = m_CommandAllocator->GetCommandPool(),
				.level = vk::CommandBufferLevel::ePrimary,
				.commandBufferCount = static_cast<uint32_t>(workCounter.MultiBufferingCount())
			};

			m_CommandBuffer = VulkanUtil::GetCheckedVkValue(device->GetDevice().allocateCommandBuffersUnique(info));
		}

		void VulkanRHICommandList::ClearRenderTarget(RHITexture::Ptr renderTarget, RenderCore::Color& color)
		{
			ENGINE_ASSERT(renderTarget != nullptr);
			VulkanRHITexture* vkRenderTarget = reinterpret_cast<VulkanRHITexture*>(renderTarget.Get());

			SetTextureState(
				vkRenderTarget,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				{},
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::ImageAspectFlagBits::eColor
			);

			m_ColorAttachmentInfo = 
			{
				.imageView = vkRenderTarget->GetImageView(),
				.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.clearValue = vk::ClearColorValue(color.x, color.y, color.z, color.w)
			};
		}

		void VulkanRHICommandList::ClearDepthStencilView(RHITexture::Ptr depthStencil, ClearFlags::Flag clearFlags, float depth, uint8_t stencil)
		{
			ENGINE_ASSERT(depthStencil != nullptr);
			VulkanRHITexture* vkDepthStencil = reinterpret_cast<VulkanRHITexture*>(depthStencil.Get());
			// NOTE - assuming depth is always there
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eDepth;

			aspect = (clearFlags & ClearFlags::Stencil)
				? aspect | vk::ImageAspectFlagBits::eStencil
				: aspect;

			vk::ImageLayout layout = (clearFlags & ClearFlags::Stencil)
				? vk::ImageLayout::eDepthStencilAttachmentOptimal
				: vk::ImageLayout::eDepthAttachmentOptimal;

			SetTextureState(
				vkDepthStencil,
				vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				vk::ImageLayout::eUndefined,
				layout,
				aspect
			);

			m_DepthAttachmentInfo = 
			{
				.imageView = vkDepthStencil->GetImageView(),

				.imageLayout = layout,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.clearValue = vk::ClearDepthStencilValue(depth, static_cast<uint32_t>(stencil))
			};
		}

		void VulkanRHICommandList::SetRenderTargets(uint32_t TargetsNum, RHITexture::Ptr renderTarget, RHITexture::Ptr depthStencil)
		{
			ENGINE_ASSERT(renderTarget != nullptr);
			ENGINE_ASSERT(depthStencil != nullptr);
			ENGINE_ASSERT(TargetsNum == 1);
			// NOTE - textures unused as everything has been set in clear values functions
			// This function will only be a signal to start rendering
			vk::RenderingInfo renderingInfo = 
			{
				.renderArea = 
				{
					.offset = {0, 0}, 
					.extent = {static_cast<uint32_t>(renderTarget->GetWidth()), static_cast<uint32_t>(renderTarget->GetHeight())}
				},
				.layerCount = 1,
				.colorAttachmentCount = TargetsNum,
				.pColorAttachments = &m_ColorAttachmentInfo,
				.pDepthAttachment = &m_DepthAttachmentInfo
			};

			GetCurrentBuffer().beginRendering(renderingInfo);
		}

		void VulkanRHICommandList::SetViewport(const Viewport& viewport)
		{
			GetCurrentBuffer().setViewport(
				0,
				vk::Viewport
				{
					.x = viewport.LeftX,
					.y = viewport.GetHeight(),
					.width = viewport.GetWidth(),
					// NOTE - Flipping viewport because of vulkan coordinate system
					.height = -viewport.GetHeight(),
					.minDepth = viewport.MinDepth,
					.maxDepth = viewport.MaxDepth
				});
		}

		void VulkanRHICommandList::SetScissorRect(const Rect& scissorRect)
		{
			GetCurrentBuffer().setScissor(
				0,
				vk::Rect2D
				{
					.offset = vk::Offset2D(scissorRect.LeftX, scissorRect.TopY),
					.extent = vk::Extent2D(scissorRect.RightX, scissorRect.BottomY)
				});
		}

		void VulkanRHICommandList::Close()
		{
			VULKAN_RHI_CHECK_RESULT(GetCurrentBuffer().end());
		}

		void VulkanRHICommandList::Reset()
		{
			m_Begun = true;

			VULKAN_RHI_CHECK_RESULT(GetCurrentBuffer().reset());
			VULKAN_RHI_CHECK_RESULT(GetCurrentBuffer().begin(vk::CommandBufferBeginInfo{}));

			m_CurrentTechnique = nullptr;
		}

		void VulkanRHICommandList::SetPipelineStateObject(RHIPipelineStateObject::Ptr pso)
		{
			VulkanRHIPipelineStateObject* pipeline = reinterpret_cast<VulkanRHIPipelineStateObject*>(pso.Get());
			GetCurrentBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());
		}

		void VulkanRHICommandList::SetMesh(RHIMesh::Ptr mesh)
		{
			ENGINE_ASSERT(mesh != nullptr);

			VulkanRHIBuffer* vertexBuffer = reinterpret_cast<VulkanRHIBuffer*>(mesh->GetVertexBuffer().Get());
			VulkanRHIBuffer* indexBuffer = reinterpret_cast<VulkanRHIBuffer*>(mesh->GetIndexBuffer().Get());
			GetCurrentBuffer().bindVertexBuffers(0, vertexBuffer->GetBuffer(), {0});
			GetCurrentBuffer().bindIndexBuffer(indexBuffer->GetBuffer(), 0, ConvertToVkIndexType(mesh->GetIndexFormat()));
		}

		void VulkanRHICommandList::SetGraphicsConstantBuffer(uint32_t ParameterIdx, RHIBuffer::Ptr buffer, uint32_t bufferOffset)
		{
			ENGINE_ASSERT(m_CurrentTechnique != nullptr);
			VulkanRHIBuffer* vkBuffer = reinterpret_cast<VulkanRHIBuffer*>(buffer.Get());

			m_DescriptorInfos.emplace_back(vk::DescriptorBufferInfo
				{
				.buffer = vkBuffer->GetBuffer(),
				.offset = bufferOffset * vkBuffer->GetDesc().ElementSize,
				// NOTE - assuming, as written in shader, that only one value is used
				.range = vkBuffer->GetDesc().ElementSize
				});

			m_DescriptorWrites.emplace_back(vk::WriteDescriptorSet
				{
				.dstSet = {},
				.dstBinding = ParameterIdx,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType =
					(GetBufferUsageFlags(vkBuffer->GetDesc().UsageFlag) & vk::BufferUsageFlagBits::eUniformBuffer)
					? vk::DescriptorType::eUniformBuffer
					: vk::DescriptorType::eStorageBuffer,
				.pBufferInfo = {}
				});
		}

		void VulkanRHICommandList::SetTechnique(RHITechnique::Ptr technique)
		{
			m_CurrentTechnique = reinterpret_cast<VulkanRHITechnique*>(technique.Get());
		}

		void VulkanRHICommandList::SetPrimitiveTopology(PrimitiveTopology topology)
		{
			// NOTE - set when creating the pipeline for now
			return;
		}

		void VulkanRHICommandList::DrawIndexedInstanced(uint32_t IndexCountPerInstance, uint32_t InstanceCount, uint32_t StartIndexLocation, int32_t BaseVertexLocation, uint32_t StartInstanceLocation)
		{
			for (std::size_t i = 0; i < m_DescriptorWrites.size(); i++) 
			{
				m_DescriptorWrites[i].pBufferInfo = &m_DescriptorInfos[i];
			}

			vk::DescriptorSet vkSet = m_Pool.CreateDescriptorSet(m_CurrentTechnique, m_DescriptorWrites);

			GetCurrentBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_CurrentTechnique->GetPipelineLayout(), 0, {vkSet}, {});

			GetCurrentBuffer().drawIndexed(
				IndexCountPerInstance, 
				InstanceCount, 
				StartIndexLocation,
				BaseVertexLocation,
				StartInstanceLocation);

			m_DescriptorInfos.clear();
			m_DescriptorWrites.clear();
		}

		RenderNativeObject VulkanRHICommandList::GetNativeObject()
		{
			return RenderNativeObject(&m_CommandBuffer[m_WorkCounter.CurrentIndex()].get());
		}

		RHICommandAllocator::Ptr VulkanRHICommandList::GetAllocator() const
		{
			return m_CommandAllocator;
		}

		void VulkanRHICommandList::SetTextureState(
			VulkanRHITexture* texture,
			vk::PipelineStageFlags2 srcStageMask,
			vk::PipelineStageFlags2 dstStageMask,
			vk::AccessFlags2 srcAccessMask,
			vk::AccessFlags2 dstAccessMask,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::ImageAspectFlags aspectFlags)
		{
			vk::ImageMemoryBarrier2 barrier = 
			{
				.srcStageMask = srcStageMask,
				.srcAccessMask = srcAccessMask,
				.dstStageMask = dstStageMask,
				.dstAccessMask = dstAccessMask,
				.oldLayout = oldLayout,
				.newLayout = newLayout,
				.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
				.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
				.image = texture->GetImage(),
				.subresourceRange = 
				{
					.aspectMask = aspectFlags,
					.baseMipLevel = 0,
					.levelCount = vk::RemainingMipLevels,
					.baseArrayLayer = 0,
					.layerCount = vk::RemainingArrayLayers
				}
			};

			vk::DependencyInfo info = 
			{
				.dependencyFlags = {},
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &barrier
			};

			GetCurrentBuffer().pipelineBarrier2(info);
		}

		vk::CommandBuffer& VulkanRHICommandList::GetCurrentBuffer()
		{
			return m_CommandBuffer[m_WorkCounter.CurrentIndex()].get();
		}
	}
}