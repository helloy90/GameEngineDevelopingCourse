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

			virtual RenderNativeObject GetNativeObject() override;
			vk::Fence GetFence() const;
		private:
			const VulkanWorkCounter& m_WorkCounter;

			std::vector<vk::UniqueFence> m_Fences;
		};
	}
}