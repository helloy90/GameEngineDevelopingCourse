#include <VulkanTransferHelper.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanTransferHelper::VulkanTransferHelper(VmaAllocator allocator, vk::DeviceSize size)
			: m_Allocator(allocator)
		{
			ENGINE_ASSERT((size % 4 == 0) && "GPU access must be aligned!");

			vk::BufferCreateInfo bufInfo =
			{
				.size = size,
				.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferSrc,
				.sharingMode = vk::SharingMode::eExclusive
			};

			VmaAllocationCreateInfo allocInfo =
			{
				.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
				.usage = VMA_MEMORY_USAGE_AUTO,
				.requiredFlags = 0,
				.preferredFlags = 0,
				.memoryTypeBits = 0,
				.pool = nullptr,
				.pUserData = nullptr,
				.priority = 0.0f
			};

			VkBuffer buf;

			VkResult result = vmaCreateBuffer(
				allocator,
				&static_cast<const VkBufferCreateInfo&>(bufInfo),
				&allocInfo,
				&buf,
				&m_Allocation,
				nullptr);

			ENGINE_ASSERTF(
				result == VK_SUCCESS,
				"Error {} occured while trying to allocate Staging Buffer",
				vk::to_string(static_cast<vk::Result>(result)));

			ENGINE_ASSERT(m_Allocation != nullptr);
			m_StagingBuffer = vk::Buffer(buf);
		}

		VulkanTransferHelper::~VulkanTransferHelper()
		{
			vmaDestroyBuffer(m_Allocator, VkBuffer(m_StagingBuffer), m_Allocation);
		}

		void VulkanTransferHelper::UploadBuffer(
			VulkanRHIBuffer::Ptr buffer,
			vk::CommandBuffer commandBuffer,
			vk::DeviceSize bufferSize,
			void* data)
		{
			std::byte* mapped;

			VkResult result = vmaMapMemory(m_Allocator, m_Allocation, reinterpret_cast<void**>(&mapped));
			ENGINE_ASSERTF(
				result == VK_SUCCESS,
				"Error {} occured while trying to map Staging Buffer memory",
				vk::to_string(static_cast<vk::Result>(result)));

			std::memcpy(mapped, data, bufferSize);

			vmaUnmapMemory(m_Allocator, m_Allocation);

			{
				vk::BufferCopy2 copy =
				{
					.srcOffset = 0,
					.dstOffset = 0,
					.size = bufferSize
				};

				vk::CopyBufferInfo2 copyInfo =
				{
					.srcBuffer = m_StagingBuffer,
					.dstBuffer = buffer->GetBuffer(),
					.regionCount = 1,
					.pRegions = &copy
				};

				commandBuffer.copyBuffer2(copyInfo);
			}
		}

		RenderNativeObject VulkanTransferHelper::GetNativeObject()
		{
			ENGINE_PANIC("Should not be called!");
			return nullptr;
		}
	}
}