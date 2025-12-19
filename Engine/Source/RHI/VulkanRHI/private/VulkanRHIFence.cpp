#include "VulkanRHIFence.h"

#include "VulkanUtil.h"

#include "VulkanRHICommandQueue.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanRHIFence::VulkanRHIFence(
			const VulkanWorkCounter& workCounter, VulkanRHIDevice::Ptr device)
			: m_WorkCounter(workCounter)
		{
			m_Fences.reserve(workCounter.multiBifferingCount());

			for (std::size_t i = 0; i < workCounter.multiBifferingCount(); i++)
			{
				m_Fences.emplace_back(
					VulkanUtil::GetCheckedVkValue(device->GetDevice().createFenceUnique(vk::FenceCreateInfo{})));
			}
		}

		void VulkanRHIFence::Sync(RHICommandQueue::Ptr commandQueue)
		{
		}

		RenderNativeObject VulkanRHIFence::GetNativeObject()
		{
			return RenderNativeObject(&m_Fences[m_WorkCounter.currentIndex()].get());
		}

		vk::Fence VulkanRHIFence::GetFence() const {
			return m_Fences[m_WorkCounter.currentIndex()].get();
		}
	}
}