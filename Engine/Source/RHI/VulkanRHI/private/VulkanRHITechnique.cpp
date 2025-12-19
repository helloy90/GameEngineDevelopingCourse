#include "VulkanRHITechnique.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanRHITechnique::VulkanRHITechnique(
			const ShaderInfo& shaderInfo,
			const InputLayout& inputLayout,
			const RootSignature& rootSignature,
			const VulkanWorkCounter& workCounter,
			vk::UniquePipelineLayout&& pipelineLayout,
			vk::UniqueDescriptorSetLayout&& descriptorSetLayout,
			ShaderStagesList&& shaderStagesList)
			: RHITechnique(shaderInfo, inputLayout, rootSignature)
			, m_WorkCounter(workCounter)
			, m_PipelineLayout(std::move(pipelineLayout))
			, m_DescriptorSetLayout(std::move(descriptorSetLayout))
			, m_ShaderStagesList(std::move(shaderStagesList))
		{
		}

		RenderNativeObject VulkanRHITechnique::GetNativeObject()
		{
			VULKAN_RHI_PANIC("No native object for Technique!");
			return nullptr;
		}

		vk::PipelineLayout VulkanRHITechnique::GetPipelineLayout() const
		{
			return m_PipelineLayout.get();
		}

		vk::DescriptorSetLayout VulkanRHITechnique::GetDescriptorSetLayout() const
		{
			return m_DescriptorSetLayout.get();
		}

		const VulkanRHITechnique::ShaderStagesList& VulkanRHITechnique::GetShaderStages() const
		{
			return m_ShaderStagesList;
		}

		const RHITechnique::InputLayout& VulkanRHITechnique::GetGeneralInputLayout() const
		{
			return m_InputLayout;
		}
		const RHITechnique::ShaderInfo& VulkanRHITechnique::GetGeneralShaderInfo() const
		{
			return m_ShaderInfo;
		}
	}
}