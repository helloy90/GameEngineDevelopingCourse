#pragma once

#if defined(_WIN32) || defined(_WIN64)

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#endif

#include <vulkan/vulkan.hpp>

namespace GameEngine
{
	namespace Render::HAL
	{
    namespace VulkanUtil 
    {
      inline std::wstring WidenString(const std::string& str)
      {
        std::vector<wchar_t> buffer(
          MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size() + 1, 0, 0));

        MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size() + 1, buffer.data(), buffer.size());

        return std::wstring(buffer.data());
      }
    }

		namespace VulkanAssert 
    {
			[[noreturn]] inline void Panic(
				const std::source_location& loc,
				std::string message)
			{
        MessageBox(
          NULL,
          std::format(
            L"Panicked at {} ({}:{}), `{}`: \n\t{}",
            VulkanUtil::WidenString(loc.file_name()),
            loc.line(),
            loc.column(),
            VulkanUtil::WidenString(loc.function_name()),
            VulkanUtil::WidenString(message)).c_str(),
          L"Vulkan error occured!",
          MB_ICONERROR | MB_OK);

        std::terminate();
			}
		}
	}
}

#if (DEBUG) || (_DEBUG)

#define VULKAN_RHI_PANIC(fmtStr, ...)																																								                     \
	GameEngine::Render::HAL::VulkanAssert::Panic(std::source_location::current(), std::format(fmtStr, ##__VA_ARGS__))

#define VULKAN_RHI_VERIFYF(expr, fmtStr, ...)																																		                     \
  do																																																								                     \
  {																																																									                     \
    if (!static_cast<bool>((expr)))																																									                     \
    {																																																								                     \
      VULKAN_RHI_PANIC("assertion '{}' failed: {}", #expr, std::format(fmtStr, ##__VA_ARGS__));									                     \
    }																																																								                     \
  } while (0)

#define VULKAN_RHI_VERIFY(expr)																																											                     \
  do																																																								                     \
  {																																																									                     \
    if (!static_cast<bool>((expr)))																																									                     \
    {																																																								                     \
      VULKAN_RHI_PANIC("assertion '{}' failed.", #expr);																													                       \
    }																																																								                     \
  } while (0)

// NOTE - circumventing [[nodiscard]] qualifier that some vulkan functions have
#define VULKAN_RHI_CHECK_RESULT(expr)																																								                     \
  do																																																								                     \
  {																																																							                         \
    vk::Result _someAbsolutelyRandomNameForResult = expr;                                                                                \
    VULKAN_RHI_VERIFYF(                                                                                                                  \
      _someAbsolutelyRandomNameForResult == vk::Result::eSuccess, "Vulkan error: {}", vk::to_string(_someAbsolutelyRandomNameForResult));\
  } while (0)

#else

#define VULKAN_RHI_PANIC(fmtStr, ...)
#define VULKAN_RHI_VERIFYF(expr, fmtStr, ...)
#define VULKAN_RHI_VERIFY(expr)

// NOTE - circumventing [[nodiscard]] qualifier that some vulkan functions have
#define VULKAN_RHI_CHECK_RESULT(expr)																																								                     \
  do																																																								                     \
  {																																																							                         \
    vk::Result _someAbsolutelyRandomNameForResult = expr;                                                                                \
  } while (0)
#endif