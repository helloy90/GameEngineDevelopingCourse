#include "VulkanDescriptorPool.h"

#include "RenderCore.h"

#include "VulkanUtil.h"

#include "VulkanRHITechnique.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		static constexpr uint32_t l_MaxDescriptorsNum = 128;
		static constexpr uint32_t l_MaxTexturesNum = RenderCore::g_MaximumRenderTargets; // no texture bindings for now
		static constexpr uint32_t l_MaxRWTexturesNum = RenderCore::g_MaximumRenderTargets; // no texture bindings for now
		static constexpr uint32_t l_MaxSamplersNum = RenderCore::g_MaximumSamplers;
		static constexpr uint32_t l_MaxBuffersNum = RenderCore::g_MaximumCBV_SRV_UAV;
		static constexpr uint32_t l_MaxRWBuffersNum = RenderCore::g_MaximumCBV_SRV_UAV;

		static constexpr std::array<vk::DescriptorPoolSize, 6> l_DefaultPoolSizes = {
			vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampledImage, .descriptorCount = l_MaxRWTexturesNum},
			vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage, .descriptorCount = l_MaxRWTexturesNum},
			vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = l_MaxTexturesNum},
			vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampler, .descriptorCount = l_MaxSamplersNum},
			vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = l_MaxBuffersNum},
			vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = l_MaxRWBuffersNum}
		};

		VulkanDescriptorPool::VulkanDescriptorPool(
			const VulkanWorkCounter& workCounter, VulkanRHIDevice::Ptr device)
			: m_WorkCounter(workCounter)
			, m_Device(device->GetDevice())
		{
			vk::DescriptorPoolCreateInfo createInfo = {
				.maxSets = l_MaxDescriptorsNum,
				.poolSizeCount = static_cast<uint32_t>(l_DefaultPoolSizes.size()),
				.pPoolSizes = l_DefaultPoolSizes.data()
			};

			m_Pools.reserve(m_WorkCounter.multiBifferingCount());

			for (std::size_t i = 0; i < m_WorkCounter.multiBifferingCount(); i++) {
				m_Pools.emplace_back(VulkanUtil::GetCheckedVkValue(device->GetDevice().createDescriptorPoolUnique(createInfo)));
			}
		}

		RenderNativeObject VulkanDescriptorPool::GetNativeObject()
		{
			return RenderNativeObject(&m_Pools[m_WorkCounter.currentIndex()].get());
		}

		vk::DescriptorPool VulkanDescriptorPool::GetCurrentDescriptorPool() const
		{
			return m_Pools[m_WorkCounter.currentIndex()].get();
		}

		void VulkanDescriptorPool::ResetCurrent() {
			VULKAN_RHI_CHECK_RESULT(m_Device.resetDescriptorPool(m_Pools[m_WorkCounter.currentIndex()].get()));
		}

		vk::DescriptorSet VulkanDescriptorPool::CreateDescriptorSet(
			const VulkanRHITechnique* technique,
			std::vector<vk::WriteDescriptorSet>& writes)
		{
			vk::DescriptorSetLayout layout = technique->GetDescriptorSetLayout();

			vk::DescriptorSetAllocateInfo descSetAllocInfo = {
				.descriptorPool = m_Pools[m_WorkCounter.currentIndex()].get(),
				.descriptorSetCount = 1,
				.pSetLayouts = &layout
			};

			std::vector<vk::DescriptorSet> descriptorSet = VulkanUtil::GetCheckedVkValue(m_Device.allocateDescriptorSets(descSetAllocInfo));

			for (vk::WriteDescriptorSet& write : writes) {
				write.dstSet = descriptorSet[0];
			}

			m_Device.updateDescriptorSets(writes, {});

			return descriptorSet[0];
		}
	}
}