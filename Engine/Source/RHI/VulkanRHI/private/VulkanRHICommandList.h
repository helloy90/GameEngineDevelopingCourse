#pragma once

#include "RHICommandList.h"

#include "VulkanWorkCounter.h"

#include "VulkanRHIDevice.h"
#include "VulkanRHITexture.h"
#include "VulkanRHICommandAllocator.h"
#include "VulkanDescriptorPool.h"
#include "VulkanRHITechnique.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanRHISwapChain;

		class VulkanRHICommandList final : public RHICommandList
		{
		public:
			using Ptr = RefCountPtr<VulkanRHICommandList>;

		public:
			VulkanRHICommandList() = delete;
			VulkanRHICommandList(
				const VulkanWorkCounter& workCounter, 
				VulkanRHIDevice::Ptr device,
				VulkanDescriptorPool& pool,
				VulkanRHISwapChain* swapchain);
			~VulkanRHICommandList() = default;

		public:
			virtual void ClearRenderTarget(RHITexture::Ptr renderTarget, RenderCore::Color& color) override;
			virtual void ClearDepthStencilView(RHITexture::Ptr depthStencil, ClearFlags::Flag clearFlags, float depth, uint8_t stencil) override;
			virtual void SetRenderTargets(uint32_t TargetsNum, RHITexture::Ptr renderTarget, RHITexture::Ptr depthStencil) override;
			virtual void SetViewport(const Viewport& viewport) override;
			virtual void SetScissorRect(const Rect& scissorRect) override;
			virtual void Close() override;
			virtual void Reset() override;

			virtual void SetPipelineStateObject(RHIPipelineStateObject::Ptr pso) override;
			virtual void SetMesh(RHIMesh::Ptr mesh) override;
			virtual void SetGraphicsConstantBuffer(uint32_t ParameterIdx, RHIBuffer::Ptr buffer, uint32_t bufferOffset = 0) override;
			virtual void SetTechnique(RHITechnique::Ptr technique) override;
			virtual void SetPrimitiveTopology(PrimitiveTopology topology) override;

			virtual void DrawIndexedInstanced(
				uint32_t IndexCountPerInstance,
				uint32_t InstanceCount,
				uint32_t StartIndexLocation,
				int32_t BaseVertexLocation,
				uint32_t StartInstanceLocation
			) override;

			virtual RenderNativeObject GetNativeObject() override;
			virtual RHICommandAllocator::Ptr GetAllocator() const override;

			void SetTextureState(
				VulkanRHITexture* texture,
				vk::PipelineStageFlags2 srcStageMask,
				vk::PipelineStageFlags2 dstStageMask,
				vk::AccessFlags2 srcAccessMask,
				vk::AccessFlags2 dstAccessMask,
				vk::ImageLayout oldLayout,
				vk::ImageLayout newLayout,
				vk::ImageAspectFlags aspectFlags
			);

			vk::CommandBuffer& GetCurrentBuffer();
			bool HasBegun() const;

		private:
			const VulkanWorkCounter& m_WorkCounter;

			VulkanDescriptorPool& m_Pool;
			vk::Device m_Device{};
			VulkanRHISwapChain* m_SwapChain = nullptr;

			VulkanRHICommandAllocator::Ptr m_CommandAllocator = nullptr;

			const VulkanRHITechnique* m_CurrentTechnique = nullptr;

			std::vector<vk::UniqueCommandBuffer> m_CommandBuffer{};

			vk::RenderingAttachmentInfo m_ColorAttachmentInfo{};
			vk::RenderingAttachmentInfo m_DepthAttachmentInfo{};
			std::vector<vk::DescriptorBufferInfo> m_DescriptorInfos{};
			std::vector<vk::WriteDescriptorSet> m_DescriptorWrites{};

			bool begun = false;
		};
	}
}