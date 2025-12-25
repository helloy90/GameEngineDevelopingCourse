#pragma once

#include <RHICommon.h>

#include <Vulkan.h>

#include <vk_mem_alloc.h>

#include <VulkanRHIBuffer.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanTransferHelper final : public RefCounter<RenderBackendResource>
		{
		public:
			using Ptr = RefCountPtr<VulkanTransferHelper>;

		public:
			VulkanTransferHelper() = delete;
			VulkanTransferHelper(VmaAllocator allocator, vk::DeviceSize size);

			~VulkanTransferHelper();

		public:
			void UploadBuffer(
				VulkanRHIBuffer::Ptr buffer,
				vk::CommandBuffer commandBuffer, 
				vk::DeviceSize bufferSize,
				void* data);

			RenderNativeObject GetNativeObject() override;

		private:
			VmaAllocator m_Allocator{};
			VmaAllocation m_Allocation{};

			vk::Buffer m_StagingBuffer{};
		};
	}
}