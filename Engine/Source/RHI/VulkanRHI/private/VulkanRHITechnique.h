#pragma once

#include "RHITechnique.h"

#include "Vulkan.h"

#include "VulkanWorkCounter.h"

namespace GameEngine
{
	namespace Render::HAL
	{

		class VulkanRHITechnique final : public RHITechnique
		{
		public:
			using Ptr = RefCountPtr<VulkanRHITechnique>;
			using ShaderStagesList = std::vector<vk::UniqueShaderModule>;

		public:
			VulkanRHITechnique() = delete;
			VulkanRHITechnique(
				const ShaderInfo& shaderInfo,
				const InputLayout& inputLayout,
				const RootSignature& rootSignature,
				const VulkanWorkCounter& workCounter,
				vk::UniquePipelineLayout&& pipelineLayout,
				vk::UniqueDescriptorSetLayout&& descriptorSetLayout,
				ShaderStagesList&& shaderStagesList
			);

		public:
			virtual RenderNativeObject GetNativeObject() override;
			vk::PipelineLayout GetPipelineLayout() const;
			vk::DescriptorSetLayout GetDescriptorSetLayout() const;
			const ShaderStagesList& GetShaderStages() const;

			const InputLayout& GetGeneralInputLayout() const;
			const ShaderInfo& GetGeneralShaderInfo() const;

		private:
			const VulkanWorkCounter& m_WorkCounter;

			vk::UniquePipelineLayout m_PipelineLayout{};
			vk::UniqueDescriptorSetLayout m_DescriptorSetLayout{};
			ShaderStagesList m_ShaderStagesList{};
		};
	}
}