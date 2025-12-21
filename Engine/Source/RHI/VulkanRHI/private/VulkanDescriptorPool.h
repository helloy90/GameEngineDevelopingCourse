#pragma once

#include <RHICommon.h>

#include <Vulkan.h>

#include <VulkanWorkCounter.h>
#include <VulkanRHIDevice.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanRHITechnique;

		class VulkanDescriptorPool final : public RefCounter<RenderBackendResource> {
		public:
			using Ptr = RefCountPtr<VulkanDescriptorPool>;

		public:
			VulkanDescriptorPool() = delete;
			VulkanDescriptorPool(
				const VulkanWorkCounter& workCounter, VulkanRHIDevice::Ptr device);

		public:
			virtual RenderNativeObject GetNativeObject() override;
			vk::DescriptorPool GetCurrentDescriptorPool() const;
			void ResetCurrent();

			vk::DescriptorSet CreateDescriptorSet(
				const VulkanRHITechnique* technique,
				std::vector<vk::WriteDescriptorSet>& writes);

		private:
			const VulkanWorkCounter& m_WorkCounter;

			vk::Device m_Device;

			std::vector<vk::UniqueDescriptorPool> m_Pools{};
		};
	}
}