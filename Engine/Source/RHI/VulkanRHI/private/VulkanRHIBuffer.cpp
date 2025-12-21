#include <VulkanRHIBuffer.h>

#include <VulkanRHICore.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanRHIBuffer::VulkanRHIBuffer(
			const Description& desc,
			VmaAllocator allocator,
			vk::BufferUsageFlags additionalUsage)
			: RHIBuffer(desc)
			, m_Allocator(allocator)
		{
			if (m_Description.UsageFlag == RHIBuffer::UsageFlag::ConstantBuffer)
			{
				// Should be multiple of 256
				m_Description.ElementSize = (m_Description.ElementSize + 255) & ~255;
			}

			vk::BufferCreateInfo bufInfo = 
			{
				.size = GetBufferSize(desc),
				.usage = (additionalUsage & vk::BufferUsageFlagBits::eVertexBuffer)
					? additionalUsage
					: (additionalUsage & vk::BufferUsageFlagBits::eIndexBuffer)
						? additionalUsage
						: GetBufferUsageFlags(desc.UsageFlag),
				.sharingMode = vk::SharingMode::eExclusive
			};

			VmaAllocationCreateInfo allocInfo = 
			{
				.flags = (desc.UsageFlag != RHIBuffer::UsageFlag::GpuReadOnly)
					?	VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT 
					: VmaAllocationCreateFlags(0),
				.usage = (desc.UsageFlag != RHIBuffer::UsageFlag::GpuReadOnly)
					? VMA_MEMORY_USAGE_AUTO
					: VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
				.requiredFlags = 0,
				.preferredFlags = 0,
				.memoryTypeBits = 0,
				.pool = nullptr,
				.pUserData = nullptr,
				.priority = 0.0f
			};

			VkBuffer buf;

			VkResult result = vmaCreateBuffer(
				m_Allocator,
				&static_cast<const VkBufferCreateInfo&>(bufInfo),
				&allocInfo,
				&buf,
				&m_Allocation,
				nullptr);

			VULKAN_RHI_VERIFYF(
				result == VK_SUCCESS,
				"Error {} occured while trying to allocate Buffer",
				vk::to_string(static_cast<vk::Result>(result)));

			m_Buffer = vk::Buffer(buf);
		}

		VulkanRHIBuffer::~VulkanRHIBuffer()
		{
			vmaDestroyBuffer(m_Allocator, VkBuffer(m_Buffer), m_Allocation);
		}

		void VulkanRHIBuffer::CopyData(int elementIndex, void* data, uint32_t DataSize)
		{
			VULKAN_RHI_VERIFY(m_Description.UsageFlag != RHIBuffer::UsageFlag::GpuReadOnly);

			std::byte* mapped;

			VkResult result = vmaMapMemory(m_Allocator, m_Allocation, reinterpret_cast<void**>(&mapped));
			VULKAN_RHI_VERIFYF(
				result == VK_SUCCESS,
				"Error {} occured while trying to map Buffer memory!",
				vk::to_string(static_cast<vk::Result>(result)));

			std::memcpy((mapped + elementIndex * m_Description.ElementSize), data, DataSize);

			vmaUnmapMemory(m_Allocator, m_Allocation);
			mapped = nullptr;
		}

		RenderNativeObject VulkanRHIBuffer::GetNativeObject()
		{
			return RenderNativeObject(&m_Buffer);
		}

		vk::Buffer VulkanRHIBuffer::GetBuffer() const {
			return m_Buffer;
		}
	}
}