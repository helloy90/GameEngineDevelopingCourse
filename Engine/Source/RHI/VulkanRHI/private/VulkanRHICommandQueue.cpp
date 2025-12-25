#include <VulkanRHICommandQueue.h>

#include <array.h>

#include <Vulkan.h>

#include <VulkanRHICommandList.h>
#include <VulkanRHIFence.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanRHICommandQueue::VulkanRHICommandQueue(VulkanRHIDevice::Ptr device, VulkanRHIFence* fence)
			: m_UniversalQueue(device->GetDevice().getQueue(device->GetUniversalQueueIdx(), 0))
			, m_Fence(fence)
		{
		}

		void VulkanRHICommandQueue::ExecuteCommandLists(const std::vector<RHICommandList::Ptr>& cmdLists)
		{
			std::vector<vk::CommandBuffer> buffers;
			buffers.reserve(cmdLists.size());

			for (std::size_t i = 0; i < cmdLists.size(); i++) 
			{
				VulkanRHICommandList* commandbuffer = reinterpret_cast<VulkanRHICommandList*>(cmdLists[i].Get());
				buffers.emplace_back(commandbuffer->GetCurrentBuffer());
			}

			vk::SubmitInfo submitInfo = {};
			submitInfo.setCommandBuffers(buffers);

			if (m_SyncObjects.available != VK_NULL_HANDLE)
			{
				Core::array<vk::PipelineStageFlags, 1> waitStages =
				{
					vk::PipelineStageFlagBits::eColorAttachmentOutput
				};
				submitInfo.setWaitDstStageMask(waitStages);

				Core::array<vk::Semaphore, 1> wait =
				{ 
						m_SyncObjects.available,
				};

				submitInfo.setWaitSemaphores(wait);

				Core::array<vk::Semaphore, 1> signal =
				{
						m_SyncObjects.readyForPresent,
				};

				submitInfo.setSignalSemaphores(signal);
			}

			VULKAN_RHI_CHECK_RESULT(m_UniversalQueue.submit({ submitInfo }, m_Fence->GetFence()));

			m_Fence->SignalCurrentFence();
		}

		void VulkanRHICommandQueue::SetSyncObjects(SyncObjects objects)
		{
			m_SyncObjects = objects;
		}

		RenderNativeObject VulkanRHICommandQueue::GetNativeObject()
		{
			return RenderNativeObject(&m_UniversalQueue);
		}

		vk::Queue& VulkanRHICommandQueue::GetQueue()
		{
			return m_UniversalQueue;
		}
	}
}