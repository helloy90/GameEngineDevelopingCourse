#pragma once

#include <GUI/export.h>

#include <RHIContext.h>

struct ImDrawData;

namespace vk {
	class DescriptorPool;
}

namespace GameEngine
{
	namespace GUI
	{
		class GUI_API VulkanRenderBackend final
		{
		public:
			VulkanRenderBackend() = delete;
			VulkanRenderBackend(VulkanRenderBackend&) = delete;
			VulkanRenderBackend operator=(VulkanRenderBackend&) = delete;

		public:
			static void Init(Render::HAL::RHIContext::Ptr rhiContext);
			static void Render(ImDrawData* drawData);
			static void NewFrame();

		private:
			static std::unique_ptr<vk::DescriptorPool> m_DescriptorPool;
		};
	}
}
