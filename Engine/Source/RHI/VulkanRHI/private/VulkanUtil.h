#pragma once

#include <Vulkan.h>

namespace GameEngine
{
	namespace Render::HAL
	{
		namespace VulkanUtil
		{
			template <class T>
			T GetCheckedVkValue(vk::ResultValue<T>&& resultVal)
			{
				VULKAN_RHI_VERIFYF(
					resultVal.result == vk::Result::eSuccess,
					"Vulkan error: {}",
					vk::to_string(resultVal.result));
				return std::move(resultVal.value);
			}
		};
	}
}