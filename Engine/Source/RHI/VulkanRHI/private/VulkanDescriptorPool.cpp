#include <VulkanDescriptorPool.h>

#include <RenderCore.h>
#include <array.h>

#include <VulkanUtil.h>

#include <VulkanRHITechnique.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		static constexpr uint32_t s_MaxDescriptorsNum = 128;
		static constexpr uint32_t s_MaxTexturesNum = RenderCore::g_MaximumRenderTargets; // no texture bindings for now
		static constexpr uint32_t s_MaxRWTexturesNum = RenderCore::g_MaximumRenderTargets; // no texture bindings for now
		static constexpr uint32_t s_MaxSamplersNum = RenderCore::g_MaximumSamplers;
		static constexpr uint32_t s_MaxBuffersNum = RenderCore::g_MaximumCBV_SRV_UAV;
		static constexpr uint32_t s_MaxRWBuffersNum = RenderCore::g_MaximumCBV_SRV_UAV;

		static constexpr Core::array<vk::DescriptorPoolSize, 6> s_DefaultPoolSizes = 
		{
			vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampledImage, .descriptorCount = s_MaxRWTexturesNum},
			vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage, .descriptorCount = s_MaxRWTexturesNum},
			vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = s_MaxTexturesNum},
			vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampler, .descriptorCount = s_MaxSamplersNum},
			vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = s_MaxBuffersNum},
			vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = s_MaxRWBuffersNum}
		};

		VulkanDescriptorPool::VulkanDescriptorPool(
			const VulkanWorkCounter& workCounter, VulkanRHIDevice::Ptr device)
			: m_WorkCounter(workCounter)
			, m_Device(device->GetDevice())
		{
			vk::DescriptorPoolCreateInfo createInfo = 
			{
				.maxSets = s_MaxDescriptorsNum,
				.poolSizeCount = static_cast<uint32_t>(s_DefaultPoolSizes.size()),
				.pPoolSizes = s_DefaultPoolSizes.begin()
			};

			m_Pools.reserve(m_WorkCounter.MultiBufferingCount());

			for (std::size_t i = 0; i < m_WorkCounter.MultiBufferingCount(); i++) 
			{
				m_Pools.emplace_back(VulkanUtil::GetCheckedVkValue(device->GetDevice().createDescriptorPoolUnique(createInfo)));
			}
		}

		RenderNativeObject VulkanDescriptorPool::GetNativeObject()
		{
			return RenderNativeObject(&m_Pools[m_WorkCounter.CurrentIndex()].get());
		}

		vk::DescriptorPool VulkanDescriptorPool::GetCurrentDescriptorPool() const
		{
			return m_Pools[m_WorkCounter.CurrentIndex()].get();
		}

		void VulkanDescriptorPool::ResetCurrent() 
		{
			VULKAN_RHI_CHECK_RESULT(m_Device.resetDescriptorPool(m_Pools[m_WorkCounter.CurrentIndex()].get()));
		}

		vk::DescriptorSet VulkanDescriptorPool::CreateDescriptorSet(
			const VulkanRHITechnique* technique,
			std::vector<vk::WriteDescriptorSet>& writes)
		{
			vk::DescriptorSetLayout layout = technique->GetDescriptorSetLayout();

			vk::DescriptorSetAllocateInfo descSetAllocInfo = 
			{
				.descriptorPool = m_Pools[m_WorkCounter.CurrentIndex()].get(),
				.descriptorSetCount = 1,
				.pSetLayouts = &layout
			};

			std::vector<vk::DescriptorSet> descriptorSet = VulkanUtil::GetCheckedVkValue(m_Device.allocateDescriptorSets(descSetAllocInfo));

			for (vk::WriteDescriptorSet& write : writes) 
			{
				write.dstSet = descriptorSet[0];
			}

			m_Device.updateDescriptorSets(writes, {});

			return descriptorSet[0];
		}
	}
}