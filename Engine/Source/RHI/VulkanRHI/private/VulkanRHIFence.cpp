#include <VulkanRHIFence.h>

#include <VulkanUtil.h>

#include <VulkanRHICommandQueue.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanRHIFence::VulkanRHIFence(
			const VulkanWorkCounter& workCounter, VulkanRHIDevice::Ptr device)
			: m_WorkCounter(workCounter)
			, m_Device(device->GetDevice())
		{
			m_Fences.reserve(workCounter.MultiBufferingCount());
			m_FenceSignaled.reserve(workCounter.MultiBufferingCount());

			for (std::size_t i = 0; i < workCounter.MultiBufferingCount(); i++)
			{
				m_Fences.emplace_back(
					VulkanUtil::GetCheckedVkValue(device->GetDevice().createFenceUnique(vk::FenceCreateInfo{})));
				m_FenceSignaled.emplace_back(false);
			}
		}

		void VulkanRHIFence::Sync(RHICommandQueue::Ptr commandQueue)
		{
			if (!CurrentFenceSignaled()) {
				return;
			}

			VULKAN_RHI_CHECK_RESULT(m_Device.waitForFences({ GetFence()}, vk::True, 1000000000));
			VULKAN_RHI_CHECK_RESULT(m_Device.resetFences({ GetFence() }));

			m_FenceSignaled[m_WorkCounter.CurrentIndex()] = false;
		}

		void VulkanRHIFence::SignalCurrentFence()
		{
			m_FenceSignaled[m_WorkCounter.CurrentIndex()] = true;
		}

		RenderNativeObject VulkanRHIFence::GetNativeObject()
		{
			return RenderNativeObject(&m_Fences[m_WorkCounter.CurrentIndex()].get());
		}

		vk::Fence VulkanRHIFence::GetFence() const
		{
			return m_Fences[m_WorkCounter.CurrentIndex()].get();
		}

		bool VulkanRHIFence::CurrentFenceSignaled() const
		{
			return m_FenceSignaled[m_WorkCounter.CurrentIndex()];
		}
	}
}