#pragma once

// NOTE - Needed definitions for DXCompiler
#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wrl.h>
#else
#include <dxc/WinAdapter.h>
#endif

// NOTE - Using DXCompiler, provided by Vulkan SDK
#include <dxc/dxcapi.h>

#include <RHICommon.h>

#include <VulkanRHIDevice.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanHLSLCompiler final {
		
		public:
			VulkanHLSLCompiler();

		public:
			RefCountPtr<IDxcBlob> CompileShader(
				const std::wstring& filename, 
				const std::string& entrypoint, 
				const std::string& target) const;

		private:
			RefCountPtr<IDxcCompiler3> compiler = nullptr;
			RefCountPtr<IDxcUtils> utils = nullptr;
		};
	}
}