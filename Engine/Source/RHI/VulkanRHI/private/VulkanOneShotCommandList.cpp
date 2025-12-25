#include <VulkanOneShotCommandList.h>

#include <array.h>

#include <VulkanUtil.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanOneShotCommandList::VulkanOneShotCommandList(
			VulkanRHIDevice::Ptr device, VulkanRHICommandQueue::Ptr queue)
			: m_Device(device->GetDevice())
			, m_SubmitQueue(queue->GetQueue())
			, m_Pool(
				VulkanUtil::GetCheckedVkValue(device->GetDevice().createCommandPoolUnique(vk::CommandPoolCreateInfo
					{
						.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
						.queueFamilyIndex = device->GetUniversalQueueIdx()
					})) )
			, m_CommandBuffer(
				std::move(VulkanUtil::GetCheckedVkValue(
					device->GetDevice().allocateCommandBuffersUnique(vk::CommandBufferAllocateInfo
						{
							.commandPool = m_Pool.get(),
							.level = vk::CommandBufferLevel::ePrimary,
							.commandBufferCount = 1
						})).front()) )
			, m_Finished(
				VulkanUtil::GetCheckedVkValue(device->GetDevice().createFenceUnique(vk::FenceCreateInfo{})))
		{
		}

		vk::CommandBuffer VulkanOneShotCommandList::Start()
		{
			return m_CommandBuffer.get();
		}

		void VulkanOneShotCommandList::SubmitAndWait(vk::CommandBuffer buffer)
		{
			ENGINE_ASSERT(buffer == m_CommandBuffer.get());

			Core::array<vk::CommandBufferSubmitInfo, 1> commandBufferSubmitInfo = 
			{
				vk::CommandBufferSubmitInfo
				{
					.commandBuffer = m_CommandBuffer.get(),
					.deviceMask = 1
				}
			};

			vk::SubmitInfo2 submitInfo{};
			submitInfo.setCommandBufferInfos(commandBufferSubmitInfo);

			VULKAN_RHI_CHECK_RESULT(m_SubmitQueue.submit2({submitInfo}, m_Finished.get()));
			VULKAN_RHI_CHECK_RESULT(m_Device.waitForFences({ m_Finished.get() }, vk::True, 1000000000));

			VULKAN_RHI_CHECK_RESULT(m_Device.resetFences({ m_Finished.get() }));
			VULKAN_RHI_CHECK_RESULT(m_CommandBuffer->reset());
		}
	}
}