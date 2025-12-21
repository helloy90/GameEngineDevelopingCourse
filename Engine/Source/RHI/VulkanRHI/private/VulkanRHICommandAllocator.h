#pragma once

#include <RHICommandAllocator.h>

#include <Vulkan.h>

#include <VulkanRHIDevice.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanRHICommandAllocator final : public RHICommandAllocator
		{
		public:
			using Ptr = RefCountPtr<VulkanRHICommandAllocator>;

		public:
			VulkanRHICommandAllocator() = delete;
			VulkanRHICommandAllocator(VulkanRHIDevice::Ptr device);
			~VulkanRHICommandAllocator() = default;

		public:
			virtual void Reset() override;
			virtual RenderNativeObject GetNativeObject() override;

			vk::CommandPool GetCommandPool() const;

		private:
			vk::UniqueCommandPool m_Pool;
		};
	}
}