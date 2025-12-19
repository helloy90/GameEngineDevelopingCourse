#include "VulkanSurface.h"

#include <Window/IWindow.h>

#include "VulkanUtil.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanSurface::VulkanSurface(VulkanRHIFactory::Ptr instance)
		{
			vk::Win32SurfaceCreateInfoKHR createInfo{
				.hinstance = reinterpret_cast<HINSTANCE>(Core::g_MainWindowsApplication->GetInstanceHandle()),
				.hwnd = reinterpret_cast<HWND>(Core::g_MainWindowsApplication->GetWindowHandle()),
			};

			m_Surface = vk::UniqueSurfaceKHR{
				VulkanUtil::GetCheckedVkValue(instance->GetInstance().createWin32SurfaceKHR(createInfo)),
				vk::detail::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>{instance->GetInstance()}
			};
		}

		vk::SurfaceKHR VulkanSurface::GetSurface() const
		{
			return m_Surface.get();
		}
	}
}