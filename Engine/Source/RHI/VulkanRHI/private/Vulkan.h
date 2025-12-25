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

#include <Debug/Assertions.h>

#include <vulkan/vulkan.hpp>


#if (DEBUG) || (_DEBUG)

// NOTE - circumventing [[nodiscard]] qualifier that some vulkan functions have
#define VULKAN_RHI_CHECK_RESULT(expr)																																								                     \
  do																																																								                     \
  {																																																							                         \
    vk::Result _someAbsolutelyRandomNameForResult = expr;                                                                                \
    ENGINE_ASSERTF(                                                                                                                  \
      _someAbsolutelyRandomNameForResult == vk::Result::eSuccess, "Vulkan error: {}", vk::to_string(_someAbsolutelyRandomNameForResult));\
  } while (0)

#else

// NOTE - circumventing [[nodiscard]] qualifier that some vulkan functions have
#define VULKAN_RHI_CHECK_RESULT(expr)																																								                     \
  do																																																								                     \
  {																																																							                         \
    vk::Result _someAbsolutelyRandomNameForResult = expr;                                                                                \
  } while (0)
#endif