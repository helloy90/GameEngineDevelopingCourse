#pragma once

#include <RHIDevice.h>

#include <Vulkan.h>

#include <VulkanRHIFactory.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanRHIDevice final : public RHIDevice
		{
		public:
			using Ptr = RefCountPtr<VulkanRHIDevice>;

		public:
			VulkanRHIDevice() = delete;
			VulkanRHIDevice(VulkanRHIFactory::Ptr instance);
			~VulkanRHIDevice() = default;

		public:
			virtual RenderNativeObject GetNativeObject() override;
			vk::PhysicalDevice GetPhysicalDevice() const;
			vk::Device GetDevice() const;
			uint32_t GetUniversalQueueIdx() const;

		private:
			vk::PhysicalDevice m_PhysDevice{};
			vk::UniqueDevice m_NativeDevice{};
			// NOTE - needed for logical device creation, 
			// so it is set here and not in VulkanRHICommandQueue
			uint32_t universalQueueIdx{};
		};
	}
}