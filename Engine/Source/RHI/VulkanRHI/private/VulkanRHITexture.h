#pragma once

#include <RHITexture.h>

#include <Vulkan.h>
#include <vk_mem_alloc.h>

#include <VulkanRHIDevice.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanRHITexture final : public RHITexture
		{
		public:
			using Ptr = RefCountPtr<VulkanRHITexture>;

		public:
			VulkanRHITexture() = delete;
			VulkanRHITexture(
				const RHITexture::Description& desc,
				vk::Image&& image,
				vk::UniqueImageView&& view);
			VulkanRHITexture(
				const RHITexture::Description& desc,
				VulkanRHIDevice::Ptr device,
				VmaAllocator allocator);

			~VulkanRHITexture();

		public:
			virtual RenderNativeObject GetNativeObject() override;
			vk::Image GetImage() const;
			vk::ImageView GetImageView() const;

		private:
			VmaAllocator m_Allocator{};
			VmaAllocation m_Allocation{};

			vk::Image m_Image{};
			vk::UniqueImageView m_View{};
		};
	}
}