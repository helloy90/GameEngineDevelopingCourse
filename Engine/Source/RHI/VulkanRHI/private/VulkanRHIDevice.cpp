#include "VulkanRHIDevice.h"

#include "VulkanUtil.h"

namespace GameEngine
{
	namespace Render::HAL
	{
		template <std::size_t Size>
		static std::string_view safeViewFromArray(const vk::ArrayWrapper1D<char, Size>& array)
		{
			const std::size_t length = static_cast<const char*>(std::memchr(array.data(), '\0', Size)) - array.data();
			return std::string_view(array.data(), length);
		}

		static bool checkPhysicalDeviceExtentionsSupport(
			vk::PhysicalDevice physDevice, const std::vector<const char*>& deviceExtentions)
		{
			std::vector availableExtentions = VulkanUtil::GetCheckedVkValue(physDevice.enumerateDeviceExtensionProperties());

			std::unordered_set<std::string_view> requestedExtentions(deviceExtentions.begin(), deviceExtentions.end());

			for (const auto& ext : availableExtentions) {
				requestedExtentions.erase(safeViewFromArray(ext.extensionName));
			}

			return requestedExtentions.empty();
		}

		static bool deviceTypeIsBetter(vk::PhysicalDeviceType first, vk::PhysicalDeviceType second)
		{
			auto score = [](vk::PhysicalDeviceType type) -> int {
				switch (type) {
				case vk::PhysicalDeviceType::eVirtualGpu:
					return 1;
				case vk::PhysicalDeviceType::eIntegratedGpu:
					return 2;
				case vk::PhysicalDeviceType::eDiscreteGpu:
					return 3;
				default:
					return 0;
				}
				};
			return (score(first) > score(second));
		}

		static vk::PhysicalDevice pickPhysicalDevice(
			vk::Instance instance, const std::vector<const char*>& deviceExtentions)
		{
			std::vector physDevices = VulkanUtil::GetCheckedVkValue(instance.enumeratePhysicalDevices());

			VULKAN_RHI_VERIFYF(!physDevices.empty(), "No GPU on this PC supports Vulkan!");

			vk::PhysicalDevice bestDevice = physDevices.front();
			vk::PhysicalDeviceProperties bestDeviceProps = physDevices.front().getProperties();

			for (const auto& physDevice : physDevices)
			{
				vk::PhysicalDeviceProperties deviceProps = physDevice.getProperties();

				if (!checkPhysicalDeviceExtentionsSupport(physDevice, deviceExtentions))
				{
					continue;
				}

				if (deviceTypeIsBetter(deviceProps.deviceType, bestDeviceProps.deviceType))
				{
					bestDevice = physDevice;
					bestDeviceProps = deviceProps;
				}
			}

			return bestDevice;
		}

		static uint32_t getQueueFamilyIndex(vk::PhysicalDevice physDevice, vk::QueueFlags flags)
		{
			std::vector queueFamilies = physDevice.getQueueFamilyProperties();

			// NOTE - assuming that graphics queue supports presentation automatically
			for (uint32_t i = 0; i < queueFamilies.size(); i++)
			{
				const vk::QueueFamilyProperties& queueProps = queueFamilies[i];

				if (queueProps.queueCount > 0 && (queueProps.queueFlags & flags))
				{
					return i;
				}
			}

			VULKAN_RHI_PANIC("Could not find queue family with all requested flags!");
		}

		static vk::UniqueDevice createLogicalDevice(
			vk::PhysicalDevice physDevice,
			uint32_t universalQueueFamily,
			const std::vector<const char*>& deviceExtentions)
		{
			const float defaultQueuePriority = 1.0f;

			const std::array queueInfos{
				vk::DeviceQueueCreateInfo{
					.queueFamilyIndex = universalQueueFamily,
					.queueCount = 1,
					.pQueuePriorities = &defaultQueuePriority
				}
			};

			vk::StructureChain
				< vk::PhysicalDeviceFeatures2
				, vk::PhysicalDeviceDescriptorIndexingFeatures
				, vk::PhysicalDeviceDynamicRenderingFeatures
				, vk::PhysicalDeviceSynchronization2Features
				> featureChain = {
				// NOTE - add optional features here if nesessary
				vk::PhysicalDeviceFeatures2{
					.features = {
						.depthClamp = vk::True,
					}
				},
				vk::PhysicalDeviceDescriptorIndexingFeatures{
				},
				{.dynamicRendering = vk::True},
				{.synchronization2 = vk::True}
			};

			vk::DeviceCreateInfo createInfo{
				.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
			};

			createInfo.setQueueCreateInfos(queueInfos);
			createInfo.setPEnabledExtensionNames(deviceExtentions);

			return VulkanUtil::GetCheckedVkValue(physDevice.createDeviceUnique(createInfo));
		}


		VulkanRHIDevice::VulkanRHIDevice(VulkanRHIFactory::Ptr instance)
		{
			std::vector<const char*> deviceExtentions{
				vk::KHRSwapchainExtensionName
			};

			m_PhysDevice = pickPhysicalDevice(instance->GetInstance(), deviceExtentions);

			constexpr vk::QueueFlags universalQueueFlags =
				vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;

			universalQueueIdx = getQueueFamilyIndex(m_PhysDevice, universalQueueFlags);

			m_NativeDevice = createLogicalDevice(m_PhysDevice, universalQueueIdx, deviceExtentions);

			VULKAN_HPP_DEFAULT_DISPATCHER.init(m_NativeDevice.get());
		}

		RenderNativeObject VulkanRHIDevice::GetNativeObject()
		{
			return RenderNativeObject(&m_NativeDevice.get());
		}

		vk::PhysicalDevice VulkanRHIDevice::GetPhysicalDevice() const
		{
			return m_PhysDevice;
		}

		vk::Device VulkanRHIDevice::GetDevice() const
		{
			return m_NativeDevice.get();
		}

		uint32_t VulkanRHIDevice::GetUniversalQueueIdx() const
		{
			return universalQueueIdx;
		}
	}
}