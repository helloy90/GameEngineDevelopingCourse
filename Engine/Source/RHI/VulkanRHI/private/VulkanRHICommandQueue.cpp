#include <VulkanRHICommandQueue.h>

#include <array.h>

#include <Vulkan.h>

#include <VulkanRHICommandList.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanRHICommandQueue::VulkanRHICommandQueue(VulkanRHIDevice::Ptr device)
			: m_UniversalQueue(device->GetDevice().getQueue(device->GetUniversalQueueIdx(), 0))
		{
		}

		void VulkanRHICommandQueue::ExecuteCommandLists(const std::vector<RHICommandList::Ptr>& cmdLists)
		{
			std::vector<vk::CommandBufferSubmitInfo> bufferSubmitInfos;
			bufferSubmitInfos.reserve(cmdLists.size());

			for (std::size_t i = 0; i < cmdLists.size(); i++) 
			{
				VulkanRHICommandList* commandbuffer = reinterpret_cast<VulkanRHICommandList*>(cmdLists[i].Get());
				// NOTE - return, if command buffer has not begun 
				// (there is no need to execute anything in vulkan at initialization)
				if (!commandbuffer->HasBegun()) 
				{
					return;
				}
				bufferSubmitInfos.emplace_back(vk::CommandBufferSubmitInfo
				{
					.commandBuffer = commandbuffer->GetCurrentBuffer(),
					.deviceMask = 0
				});
			}

			Core::array<vk::SemaphoreSubmitInfo, 1> wait = 
			{ vk::SemaphoreSubmitInfo
				{
					.semaphore = m_SyncObjects.available,
					.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
					.deviceIndex = 0
				}
			};

			vk::SubmitInfo2 submitInfo = {};
			submitInfo.setCommandBufferInfos(bufferSubmitInfos);
			submitInfo.setWaitSemaphoreInfos(wait);

			VULKAN_RHI_CHECK_RESULT(m_UniversalQueue.submit2({ submitInfo }, m_SyncObjects.commandsComplete));
		}

		void VulkanRHICommandQueue::SetSyncObjects(SyncObjects objects)
		{
			m_SyncObjects = 
			{
				.available = objects.available,
				.commandsComplete = objects.commandsComplete
			};
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