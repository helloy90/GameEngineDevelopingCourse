#pragma once

#include <RHIPipelineStateObject.h>

#include <Vulkan.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanRHIPipelineStateObject final : public RHIPipelineStateObject
		{
		public:
			using Ptr = RefCountPtr<VulkanRHIPipelineStateObject>;

		public:
			VulkanRHIPipelineStateObject() = delete;
			VulkanRHIPipelineStateObject(const Description& desc, vk::UniquePipeline&& pipeline);

			~VulkanRHIPipelineStateObject() = default;

		public:
			virtual RenderNativeObject GetNativeObject() override;
			vk::Pipeline GetPipeline() const;

		private:
			vk::UniquePipeline m_Pipeline{};
		};
	}
}