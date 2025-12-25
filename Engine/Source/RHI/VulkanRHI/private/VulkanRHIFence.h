#pragma once

#include <RHIFence.h>

#include <VulkanRHIDevice.h>
#include <VulkanWorkCounter.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanRHIFence final : public RHIFence
		{
		public:
			using Ptr = RefCountPtr<VulkanRHIFence>;

		public:
			VulkanRHIFence() = delete;
			VulkanRHIFence(const VulkanWorkCounter& workCounter, VulkanRHIDevice::Ptr device);

		public:
			virtual void Sync(RHICommandQueue::Ptr commandQueue) override;

			void SignalCurrentFence();

			virtual RenderNativeObject GetNativeObject() override;
			vk::Fence GetFence() const;
			bool CurrentFenceSignaled() const;
		private:
			const VulkanWorkCounter& m_WorkCounter;

			vk::Device m_Device{};

			std::vector<vk::UniqueFence> m_Fences{};

			std::vector<bool> m_FenceSignaled{};
		};
	}
}