#pragma once

#include "RHICommon.h"

#include "Vulkan.h"
#include "vk_mem_alloc.h"

#include "VulkanRHIFactory.h"
#include "VulkanRHIDevice.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanMemoryAllocator final : public RefCounter<RenderBackendResource>
		{
		public:
			using Ptr = RefCountPtr<VulkanMemoryAllocator>;

		public:
			VulkanMemoryAllocator(VulkanRHIFactory::Ptr instance, VulkanRHIDevice::Ptr device);

		public:
			virtual RenderNativeObject GetNativeObject() override;
			VmaAllocator GetAllocator() const;

		private:
			std::unique_ptr<VmaAllocator_T, void(*)(VmaAllocator)> m_Allocator{nullptr, nullptr};
		};
	}
}