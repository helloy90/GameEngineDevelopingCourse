#include "VulkanRHIPipelineStateObject.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanRHIPipelineStateObject::VulkanRHIPipelineStateObject(const Description& desc, vk::UniquePipeline&& pipeline)
			: RHIPipelineStateObject(desc)
			, m_Pipeline(std::move(pipeline))
		{
		}

		RenderNativeObject VulkanRHIPipelineStateObject::GetNativeObject()
		{
			return RenderNativeObject(&m_Pipeline.get());
		}

		vk::Pipeline VulkanRHIPipelineStateObject::GetPipeline() const {
			return m_Pipeline.get();
		}
	}
}