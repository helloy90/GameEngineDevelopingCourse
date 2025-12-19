#include "VulkanHLSLCompiler.h"

#include <Debug/Console.h>

#include "VulkanUtil.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		VulkanHLSLCompiler::VulkanHLSLCompiler()
		{
			HRESULT hres;
			hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
			assert(SUCCEEDED(hres));

			hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
			assert(SUCCEEDED(hres));
		}

		RefCountPtr<IDxcBlob> VulkanHLSLCompiler::CompileShader(
			VulkanRHIDevice::Ptr device,
			const std::wstring& filename, 
			const std::string& entrypoint, 
			const std::string& target) const
		{
			uint32_t codePage = DXC_CP_ACP;

			RefCountPtr<IDxcBlobEncoding> sourceBlob;

			HRESULT hres = utils->LoadFile(filename.c_str(), &codePage, &sourceBlob);

			assert(SUCCEEDED(hres));

			std::wstring entrypointWStr = VulkanUtil::widenString(entrypoint);
			std::wstring targetWStr = VulkanUtil::widenString(target);

			std::vector<LPCWSTR> arguments = {
				L"-spirv",
				L"-T", targetWStr.c_str(),
				L"-E", entrypointWStr.c_str(),
				filename.c_str()
			};

			DxcBuffer buffer = {
				.Ptr = sourceBlob->GetBufferPointer(),
				.Size = sourceBlob->GetBufferSize(),
				.Encoding = DXC_CP_ACP
			};

			RefCountPtr<IDxcResult> result = nullptr;

			hres = compiler->Compile(
				&buffer,
				arguments.data(),
				static_cast<uint32_t>(arguments.size()),
				nullptr,
				IID_PPV_ARGS(&result));

			if (SUCCEEDED(hres))
			{
				result->GetStatus(&hres);
			}

			if (FAILED(hres) && (result))
			{
				RefCountPtr<IDxcBlobEncoding> errorBlob;
				hres = result->GetErrorBuffer(&errorBlob);
				assert(SUCCEEDED(hres));
				Core::Console::PrintDebug("Shader compilation failed: \n\n ", static_cast<const char*>(errorBlob->GetBufferPointer()));

				assert(false && "Compilation failed");
			}

			RefCountPtr<IDxcBlob> code;
			result->GetResult(&code);
			std::filesystem::path filepath = std::filesystem::path(filename);
			filepath.replace_filename(std::format("Object_{}", entrypoint));
			filepath.replace_extension(".spv");

			return code;
		}

		RenderNativeObject VulkanHLSLCompiler::GetNativeObject()
		{
			assert(false && "No native object should be provided for compiler!");
			return nullptr;
		}
	}
}