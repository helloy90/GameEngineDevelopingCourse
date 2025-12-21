#include <VulkanRHITexture.h>

#include <VulkanRHICore.h>

#include <VulkanUtil.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanRHITexture::VulkanRHITexture(
			const RHITexture::Description& desc, 
			vk::Image&& image,
			vk::UniqueImageView&& view)
				: RHITexture(desc)
				, m_Image(std::move(image))
				, m_View(std::move(view))
		{
		}

		VulkanRHITexture::VulkanRHITexture(
			const RHITexture::Description& desc,
			VulkanRHIDevice::Ptr device,
			VmaAllocator allocator
		)	: RHITexture(desc)
			, m_Allocator(allocator)
		{
			vk::Format format = ConvertToVkFormat(desc.Format);

			vk::ImageCreateInfo imageInfo = 
			{
				.flags = {},
				.imageType = GetImageType(desc.Dimension),
				.format = format,
				.extent = GetImageExtent(desc),
				.mipLevels = static_cast<uint32_t>(desc.MipLevels),
				.arrayLayers = 1,
				.samples = vk::SampleCountFlagBits::e1,
				.tiling = vk::ImageTiling::eOptimal,
				.usage = GetImageUsageFlags(desc.Flags),
				.sharingMode = vk::SharingMode::eExclusive,
				.initialLayout = vk::ImageLayout::eUndefined
			};

			VmaAllocationCreateInfo allocInfo = 
			{
				.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
				.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
				.requiredFlags = 0,
				.preferredFlags = 0,
				.memoryTypeBits = 0,
				.pool = nullptr,
				.pUserData = nullptr,
				.priority = 0.0f
			};

			VkImage img;

			VkResult result = vmaCreateImage(
				m_Allocator,
				&static_cast<const VkImageCreateInfo&>(imageInfo),
				&allocInfo,
				&img,
				&m_Allocation,
				nullptr);

			VULKAN_RHI_VERIFYF(
				result == VK_SUCCESS,
				"Error {} occured while trying to allocate Texture",
				vk::to_string(static_cast<vk::Result>(result)));

			m_Image = vk::Image(img);

			// Creating default view
			vk::ImageViewCreateInfo viewInfo = 
			{
				.image = m_Image,
				.viewType = GetImageViewType(desc.Dimension),
				.format = ConvertToVkFormat(desc.Format),
				.subresourceRange = vk::ImageSubresourceRange
				{
					.aspectMask = GetImageAspectFlags(format),
					.baseMipLevel = 0,
					.levelCount = vk::RemainingMipLevels,
					.baseArrayLayer = 0,
					.layerCount = vk::RemainingArrayLayers
				}
			};

			m_View = VulkanUtil::GetCheckedVkValue(device->GetDevice().createImageViewUnique(viewInfo));
		}

		VulkanRHITexture::~VulkanRHITexture()
		{
			// almost all images have allocation, except swapchain images
			if (m_Allocation != nullptr)
			{
				assert(m_Allocator != nullptr);
				vmaDestroyImage(m_Allocator, VkImage(m_Image), m_Allocation);
			}
		}

		RenderNativeObject VulkanRHITexture::GetNativeObject()
		{
			return RenderNativeObject(&m_Image);
		}

		vk::Image VulkanRHITexture::GetImage() const
		{
			return m_Image;
		}

		vk::ImageView VulkanRHITexture::GetImageView() const
		{
			return m_View.get();
		}
	}
}