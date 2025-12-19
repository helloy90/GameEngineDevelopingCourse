#pragma once

#include <RHIFactory.h>

#include "Vulkan.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanRHIFactory final : public RHIFactory
		{
		public:
			using Ptr = RefCountPtr<VulkanRHIFactory>;

		public:
			VulkanRHIFactory();
			~VulkanRHIFactory() = default;

		public:
			// NOTE - returns the object, constructed from *vk::Instance
			virtual RenderNativeObject GetNativeObject() override;
			vk::Instance GetInstance() const;

		private:
			vk::UniqueInstance m_NativeInstance{};
			vk::UniqueDebugUtilsMessengerEXT m_debugCallBack{};
		};
	}
}