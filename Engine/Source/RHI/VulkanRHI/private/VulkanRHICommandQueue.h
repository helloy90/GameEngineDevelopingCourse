#pragma once

#include <Vulkan.h>

#include <RHICommandQueue.h>

#include <VulkanRHIDevice.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanRHICommandQueue final : public RHICommandQueue
		{
		public:
			using Ptr = RefCountPtr<VulkanRHICommandQueue>;

		public:
			// NOTE - set in SwapChain::MakeBackBufferPresentable, used here
			struct SyncObjects {
				vk::Semaphore available;

				vk::Fence commandsComplete;
			};

		public:
			VulkanRHICommandQueue() = delete;
			VulkanRHICommandQueue(VulkanRHIDevice::Ptr device);
			~VulkanRHICommandQueue() = default;

		public:
			virtual void ExecuteCommandLists(const std::vector<RHICommandList::Ptr>& cmdLists) override;

			void SetSyncObjects(SyncObjects objects);

			virtual RenderNativeObject GetNativeObject() override;
			vk::Queue& GetQueue();

		private:
			// NOTE - using one queue for every operation (graphics, transfer, etc.)
			vk::Queue m_UniversalQueue{};

			SyncObjects m_SyncObjects;
		};
	}
}