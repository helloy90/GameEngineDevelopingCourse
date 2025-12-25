#pragma once

#if defined(_WIN32) || defined(_WIN64)

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

namespace GameEngine
{
  namespace Core
  {
    inline std::wstring WidenString(const std::string& str)
    {
      std::vector<wchar_t> buffer(
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size() + 1, 0, 0));

      MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size() + 1, buffer.data(), buffer.size());

      return std::wstring(buffer.data());
    }

    namespace Assert
    {
      [[noreturn]] inline void Panic(
        const std::source_location& loc,
        std::string message)
      {
        MessageBox(
          NULL,
          std::format(
            L"Panicked at {} ({}:{}), `{}`: \n\t{}",
            WidenString(loc.file_name()),
            loc.line(),
            loc.column(),
            WidenString(loc.function_name()),
            WidenString(message)).c_str(),
          L"Error occured!",
          MB_ICONERROR | MB_OK);

        std::terminate();
      }
    }
  }
}
#endif

#if (DEBUG) || (_DEBUG)
// NOTE - unfortunately __VA_OPT__(, ) does not work for some reason, so format string always needed
#define ENGINE_PANICF(fmtStr, ...)																																								             \
	GameEngine::Core::Assert::Panic(std::source_location::current(), std::format(fmtStr, ##__VA_ARGS__))

#define ENGINE_PANIC(msg)                                                                                                       \
    ENGINE_PANICF("{}", msg)
// Do-While requires to use ;
#define ENGINE_ASSERT_NOT_IMPLEMENTED do { ENGINE_PANIC("NOT IMPLEMENTED!!!"); } while(false)

#define ENGINE_ASSERTF(expr, fmtStr, ...)																																		                     \
  do																																																								                     \
  {																																																									                     \
    if (!static_cast<bool>((expr)))																																									                     \
    {																																																								                     \
      ENGINE_PANICF("assertion '{}' failed: {}", #expr, std::format(fmtStr, ##__VA_ARGS__));									                     \
    }																																																								                     \
  } while (0)

#define ENGINE_ASSERT(expr)																																											                     \
  do																																																								                     \
  {																																																									                     \
    if (!static_cast<bool>((expr)))																																									                     \
    {																																																								                     \
      ENGINE_PANICF("assertion '{}' failed.", #expr);																													                       \
    }																																																								                     \
  } while (0)
#else

#define ENGINE_PANICF(fmtStr, ...)
#define ENGINE_PANIC(msg)
#define ENGINE_ASSERT_NOT_IMPLEMENTED
#define ENGINE_ASSERTF(expr, fmtStr, ...)
#define ENGINE_ASSERT(expr)

#endif
