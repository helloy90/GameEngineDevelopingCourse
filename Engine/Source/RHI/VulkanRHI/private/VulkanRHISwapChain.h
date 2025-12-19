#pragma once

#include "RHISwapChain.h"

#include "Vulkan.h"

#include "VulkanSurface.h"

#include "VulkanWorkCounter.h"

#include "VulkanRHIDevice.h"
#include "VulkanRHIFactory.h"
#include "VulkanRHICommandQueue.h"
#include "VulkanRHIFence.h"
#include "VulkanRHITexture.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanRHICommandList;

		class VulkanRHISwapChain final : public RHISwapChain {
		public:
			using Ptr = RefCountPtr<VulkanRHISwapChain>;

		public:
			VulkanRHISwapChain() = delete;
			VulkanRHISwapChain(
				VulkanWorkCounter& workCounter,
				VulkanRHIFactory::Ptr instance,
				VulkanRHIDevice::Ptr device,
				VulkanRHICommandQueue::Ptr queue,
				VulkanRHIFence::Ptr fence
			);
			~VulkanRHISwapChain() = default;

		public:
			void AcquireNext();
			virtual void MakeBackBufferPresentable(RHICommandList::Ptr commandList) override;
			virtual void Present() override;

			virtual RenderNativeObject GetNativeObject() override;
			vk::SwapchainKHR GetSwapChain() const;

			virtual void Resize(RHIDevice::Ptr device, uint32_t width, uint32_t height) override;
			virtual RHITexture::Ptr GetCurrentBackBuffer() override;

		private:
			void recreateSwapChain(vk::Extent2D resolution);

			vk::Semaphore& GetImageAvailableSem();
			vk::Semaphore& GetImageReadyForPresentSem();

		private:
			struct SwapChainData {
				vk::UniqueSwapchainKHR swapchain;
				vk::Format format;
				vk::Extent2D extent;

				std::vector<VulkanRHITexture::Ptr> elements;
				std::vector<vk::UniqueSemaphore> imageAvailable;
				std::vector<vk::UniqueSemaphore> imageReadyToPresent;
			};

		private:
			VulkanWorkCounter& m_WorkCounter;
			// NOTE - saving queue ptr here for being able to set semaphores and fence for command buffer submittion
			VulkanRHICommandQueue* m_Queue = nullptr;
			// NOTE - saving fences ptr here for being able to pipe current fence to CommandQueue
			VulkanRHIFence* m_Fence = nullptr;

			VulkanSurface m_Surface;

			vk::PhysicalDevice m_PhysicalDevice;
			vk::Device m_Device;
			std::uint32_t m_QueueFamily;

			bool m_UseVsync = true;

			SwapChainData m_CurrentSwapChain{};

			uint32_t m_CurrentSemaphoreIndex = 0;
			uint32_t m_ImageIndex = 0;

			bool initialized = false;
		};
	}
}