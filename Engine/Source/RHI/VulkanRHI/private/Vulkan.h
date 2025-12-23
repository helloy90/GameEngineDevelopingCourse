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
			inline void Panic(
				const std::source_location& loc,
				std::string message)
			{
        assert(false && std::format(
          L"Vulkan error occured! Panicked at {} ({}:{}), `{}`: \n\t{}",
          VulkanUtil::WidenString(loc.file_name()),
          loc.line(),
          loc.column(),
          VulkanUtil::WidenString(loc.function_name()),
          VulkanUtil::WidenString(message)).c_str());
			}
		}
	}
}

#define VULKAN_RHI_PANIC(fmtStr, ...)																																								                     \
	GameEngine::Render::HAL::VulkanAssert::Panic(std::source_location::current(), std::format(fmtStr, ##__VA_ARGS__))

// NOTE - these macroses are used for checks that should happen even in release builds
#define VULKAN_RHI_VERIFYF(expr, format_str, ...)																																		                     \
  do																																																								                     \
  {																																																									                     \
    if (!static_cast<bool>((expr)))																																									                     \
    {																																																								                     \
      VULKAN_RHI_PANIC("assertion '{}' failed: {}", #expr, std::format(format_str, ##__VA_ARGS__));									                     \
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
