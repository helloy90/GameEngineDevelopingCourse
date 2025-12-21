#pragma once

#include <RHIContext.h>
#include <RHI/VulkanRHI/export.h>

namespace vk {
	class PhysicalDevice;
}

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanWorkCounter;

		class VulkanRHIFactory;
		class VulkanRHIDevice;
		class VulkanRHICommandQueue;
		class VulkanRHISwapChain;

		class VulkanRHIFence;
		class VulkanRHICommandList;

		class VulkanDescriptorPool;
		class VulkanHLSLCompiler;
		class VulkanMemoryAllocator;
		class VulkanOneShotCommandList;

		class VULKAN_API VulkanRHIContext final : public RHIContext
		{
		public:
			VulkanRHIContext();
			~VulkanRHIContext();

		public:
			RHIBuffer::Ptr CreateVertexBuffer(RHIBuffer::Description&& description);
			RHIBuffer::Ptr CreateIndexBuffer(RHIBuffer::Description&& description);

		public:
			virtual RHITexture::Ptr CreateTexture(const RHITexture::Description& description) override;
			virtual RHIBuffer::Ptr CreateBuffer(RHIBuffer::Description&& description) override;
			virtual RHITechnique::Ptr CreateTechnique(
				const RHITechnique::ShaderInfo& shaderInfo,
				const RHITechnique::InputLayout& inputLayout,
				const RHITechnique::RootSignature& rootSignature
			) override;
			virtual RHIPipelineStateObject::Ptr CreatePSO(const RHIPipelineStateObject::Description& description) override;
			virtual RHIMesh::Ptr CreateMesh(
				const RHIMesh::VertexBufferDescription& vertexDesc,
				const RHIMesh::IndexBufferDescription& indexDesc
			) override;
			virtual void SetDescriptorHeaps() override;

		public:
			virtual RHIDevice::Ptr GetDevice() const override;
			virtual RHIFactory::Ptr GetFactory() const override;
			virtual RHISwapChain::Ptr GetSwapChain() const override;
			virtual RHIFence::Ptr GetFence() const override;
			virtual RHICommandQueue::Ptr GetCommandQueue() const override;
			virtual RHICommandList::Ptr GetCommandList() const override;

			vk::PhysicalDevice GetPhysicalDevice() const;
			uint32_t GetQueueIdx() const;

		private:
			std::unique_ptr<VulkanWorkCounter> m_WorkCounter;

			RefCountPtr<VulkanRHIFactory> m_Instance = nullptr;
			RefCountPtr<VulkanRHIDevice> m_Device = nullptr;
			RefCountPtr<VulkanMemoryAllocator> m_MemoryAllocator = nullptr;

			RefCountPtr<VulkanRHIFence> m_Fence = nullptr;
			RefCountPtr<VulkanRHICommandQueue> m_CommandQueue = nullptr;
			RefCountPtr<VulkanDescriptorPool> m_DescriptorPool = nullptr;

			RefCountPtr<VulkanRHISwapChain> m_SwapChain = nullptr;
			RefCountPtr<VulkanRHICommandList> m_CommandBuffer = nullptr;

			std::unique_ptr<VulkanHLSLCompiler> m_HLSLCompiler = nullptr;
			
			std::unique_ptr<VulkanOneShotCommandList> m_OneShotCommandList;
		};
	}
}