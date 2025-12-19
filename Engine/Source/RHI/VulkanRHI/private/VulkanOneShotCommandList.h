#pragma once

#include "Vulkan.h"

#include "VulkanRHIDevice.h"
#include "VulkanRHICommandQueue.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanOneShotCommandList final
		{
		public:
			VulkanOneShotCommandList() = delete;
			VulkanOneShotCommandList(VulkanRHIDevice::Ptr device, VulkanRHICommandQueue::Ptr queue);

		public:
			vk::CommandBuffer Start();

			void SubmitAndWait(vk::CommandBuffer buffer);

		private:
			vk::Device m_Device;
			vk::Queue m_SubmitQueue;

			vk::UniqueCommandPool m_Pool;
			vk::UniqueCommandBuffer m_CommandBuffer;
			vk::UniqueFence m_Finished;
		};
	}
}