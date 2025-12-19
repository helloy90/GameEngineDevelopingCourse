#include "VulkanRHIFactory.h"

#include <Debug/Console.h>

#include "VulkanUtil.h"


namespace GameEngine
{
	namespace Render::HAL
	{
		static vk::Bool32 debugCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT /*messageTypes*/,
			const vk::DebugUtilsMessengerCallbackDataEXT* callbackData,
			void* /*pUserData*/)
		{
			std::ofstream out("debug.txt", std::ios_base::ate | std::ios_base::out | std::ios_base::app);
			if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
			{
				VULKAN_RHI_PANIC("Vulkan error! \n{}\n", callbackData->pMessage);
				Core::Console::PrintDebug("Vulkan error! \n{}\n", callbackData->pMessage);
			}
			else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
			{
				Core::Console::PrintDebug("Vulkan warning! \n{}\n", callbackData->pMessage);
			}
			else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
			{
				Core::Console::PrintDebug("Vulkan info: \n{}\n", callbackData->pMessage);
			}
			else
			{
				Core::Console::PrintDebug("Vulkan Verbose: \n{}\n", callbackData->pMessage);
			}

			return vk::False;
		}

		static vk::UniqueInstance createInstance(
			const std::vector<const char*> layers,
			const std::vector<const char*> instanceExtensions)
		{
			constexpr vk::ApplicationInfo appInfo{
				.pApplicationName = "Game",
				.applicationVersion = VK_MAKE_VERSION(0, 1, 0),
				.pEngineName = "Engine",
				.engineVersion = VK_MAKE_VERSION(0, 1, 0),
				.apiVersion = vk::ApiVersion14
			};

			vk::InstanceCreateInfo createInfo{
				.pApplicationInfo = &appInfo
			};

			createInfo.setPEnabledLayerNames(layers);
			createInfo.setPEnabledExtensionNames(instanceExtensions);

			return VulkanUtil::GetCheckedVkValue(vk::createInstanceUnique(createInfo));
		}

		VulkanRHIFactory::VulkanRHIFactory()
		{
			VULKAN_HPP_DEFAULT_DISPATCHER.init();

			std::vector<const char*> layers = {
				"VK_LAYER_KHRONOS_validation"
			};


			std::vector<const char*> instanceExtensions = {
#if defined(_WIN32) || defined(_WIN64)
				vk::KHRWin32SurfaceExtensionName,
#endif
				vk::KHRSurfaceExtensionName,
				vk::EXTDebugUtilsExtensionName
			};

			m_NativeInstance = createInstance(layers, instanceExtensions);

			VULKAN_HPP_DEFAULT_DISPATCHER.init(m_NativeInstance.get());

#ifdef DEBUG
			{
				vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{
					.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
						vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
						vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
						vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
					.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
						vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
						vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
					.pfnUserCallback = debugCallback,
					.pUserData = nullptr
				};

				m_debugCallBack = VulkanUtil::GetCheckedVkValue(
					m_NativeInstance->createDebugUtilsMessengerEXTUnique(debugUtilsCreateInfo));
			}
#endif // DEBUG
		}

		RenderNativeObject VulkanRHIFactory::GetNativeObject()
		{
			return RenderNativeObject(&m_NativeInstance.get());
		}

		vk::Instance VulkanRHIFactory::GetInstance() const
		{
			return m_NativeInstance.get();
		}
	}
}