#pragma once

#include "RHIBuffer.h"

#include "Vulkan.h"

#include "vk_mem_alloc.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanRHIBuffer final : public RHIBuffer
		{
		public:
			using Ptr = RefCountPtr<VulkanRHIBuffer>;

		public:
			VulkanRHIBuffer() = delete;
			VulkanRHIBuffer(
				const Description& desc, 
				VmaAllocator allocator, 
				vk::BufferUsageFlags additionalUsage = static_cast<vk::BufferUsageFlagBits>(0));

			~VulkanRHIBuffer();

		public:
			virtual void CopyData(int elementIndex, void* data, uint32_t DataSize) override;
			virtual RenderNativeObject GetNativeObject() override;

			vk::Buffer GetBuffer() const;

		private:
			VmaAllocator m_Allocator{};

			VmaAllocation m_Allocation{};
			vk::Buffer m_Buffer{};
		};
	}
}