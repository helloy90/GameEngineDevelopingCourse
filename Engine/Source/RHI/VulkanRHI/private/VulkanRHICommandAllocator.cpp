#include <VulkanRHICommandAllocator.h>

#include <VulkanUtil.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanRHICommandAllocator::VulkanRHICommandAllocator(VulkanRHIDevice::Ptr device)
			: m_Pool( VulkanUtil::GetCheckedVkValue(device->GetDevice().createCommandPoolUnique(vk::CommandPoolCreateInfo{
					.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
					.queueFamilyIndex = device->GetUniversalQueueIdx()
				})))
		{
		}

		void VulkanRHICommandAllocator::Reset()
		{
			// NOTE - noop in vulkan, no need to reset the command pool in update,
			// each individual command buffer is already being reset
			return;
		}

		RenderNativeObject VulkanRHICommandAllocator::GetNativeObject()
		{
			return RenderNativeObject(&m_Pool.get());
		}

		vk::CommandPool VulkanRHICommandAllocator::GetCommandPool() const {
			return m_Pool.get();
		}
	}
}