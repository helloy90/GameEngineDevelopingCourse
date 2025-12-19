#pragma once

#include "Vulkan.h"

#include "VulkanRHIFactory.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanSurface final
		{ 
		public:
			VulkanSurface() = delete;
			VulkanSurface(VulkanRHIFactory::Ptr instance);
			~VulkanSurface() = default;

		public:
			vk::SurfaceKHR GetSurface() const;
		private:
			vk::UniqueSurfaceKHR m_Surface{};
		};
	}
}