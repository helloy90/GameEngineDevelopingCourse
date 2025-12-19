#include "VulkanMemoryAllocator.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanMemoryAllocator::VulkanMemoryAllocator(VulkanRHIFactory::Ptr instance, VulkanRHIDevice::Ptr device)
		{
			VmaVulkanFunctions functions = {};
			functions.vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
			functions.vkGetDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;

			VmaAllocatorCreateInfo createInfo{
				.flags = {},
				.physicalDevice = device->GetPhysicalDevice(),
				.device = device->GetDevice(),
				.preferredLargeHeapBlockSize = {},
				.pAllocationCallbacks = {},
				.pDeviceMemoryCallbacks = {},
				.pHeapSizeLimit = {},
				.pVulkanFunctions = &functions,
				.instance = instance->GetInstance(),
				.vulkanApiVersion = vk::ApiVersion14,
				.pTypeExternalMemoryHandleTypes = nullptr
			};

			VmaAllocator allocator;
			::vmaCreateAllocator(&createInfo, &allocator);

			m_Allocator = { allocator, &::vmaDestroyAllocator };
		}

		RenderNativeObject VulkanMemoryAllocator::GetNativeObject()
		{
			VULKAN_RHI_PANIC("Should not be called!");
			return nullptr;
		}

		VmaAllocator VulkanMemoryAllocator::GetAllocator() const
		{
			return m_Allocator.get();
		}
	}
}