#include <VulkanRHISwapChain.h>

#include <RenderCore.h>
#include "Window/IWindow.h"

#include <VulkanUtil.h>

#include <VulkanRHICommandList.h>


namespace GameEngine
{
	namespace Render::HAL
	{
		static vk::SurfaceFormatKHR ChooseSurfaceFormat(const vk::PhysicalDevice& physDevice, const vk::SurfaceKHR& surface)
		{
			std::vector<vk::SurfaceFormatKHR> formats = VulkanUtil::GetCheckedVkValue(physDevice.getSurfaceFormatsKHR(surface));

			VULKAN_RHI_VERIFYF(!formats.empty(), "Device does not support any surface formats!");

			vk::SurfaceFormatKHR selected = formats[0];

			// NOTE - using this format for auto gamma correction
			const vk::Format desiredFormat = vk::Format::eB8G8R8A8Unorm;

			std::vector<vk::SurfaceFormatKHR>::iterator correctFormatIter = std::ranges::find_if(
				formats.begin(), formats.end(), [&desiredFormat](const vk::SurfaceFormatKHR& format) -> bool 
				{
					return format.format == desiredFormat && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
				});

			if (correctFormatIter != formats.end())
			{
				selected = *correctFormatIter;
			}

			return selected;
		}

		static vk::PresentModeKHR ChoosePresentMode(const vk::PhysicalDevice& physDevice, const vk::SurfaceKHR& surface, bool useVsync)
		{
			std::vector<vk::PresentModeKHR> modes = VulkanUtil::GetCheckedVkValue(physDevice.getSurfacePresentModesKHR(surface));

			VULKAN_RHI_VERIFYF(!modes.empty(), "Device does not support any present modes!");

			vk::PresentModeKHR selected = vk::PresentModeKHR::eFifo;

			if (!useVsync && std::ranges::find(modes.begin(), modes.end(), vk::PresentModeKHR::eMailbox) != modes.end())
			{
				selected = vk::PresentModeKHR::eMailbox;
			}

			return selected;
		}

		static vk::Extent2D ChooseSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities, vk::Extent2D resolution)
		{
			if (capabilities.currentExtent.width != (std::numeric_limits<std::uint32_t>::max)())
			{
				return capabilities.currentExtent;
			}

			return 
			{
				std::clamp<uint32_t>(
					resolution.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
				std::clamp<uint32_t>(
					resolution.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
			};
		}

		VulkanRHISwapChain::VulkanRHISwapChain(
			VulkanWorkCounter& workCounter,
			VulkanRHIFactory::Ptr instance,
			VulkanRHIDevice::Ptr device,
			VulkanRHICommandQueue::Ptr queue,
			VulkanRHIFence::Ptr fence)
			: m_WorkCounter(workCounter)
			, m_Queue(queue.Get())
			, m_Fence(fence.Get())
			, m_Surface(instance)
			, m_PhysicalDevice(device->GetPhysicalDevice())
			, m_Device(device->GetDevice())
			, m_QueueFamily(device->GetUniversalQueueIdx())
		{
			m_CurrentBackBufferIdx = m_WorkCounter.CurrentIndex();

			Resize(device, 1, 1);
		}

		void VulkanRHISwapChain::AcquireNext()
		{
			vk::AcquireNextImageInfoKHR info = 
			{
				.swapchain = m_CurrentSwapChain.swapchain.get(),
				.timeout = 1000000000,
				.semaphore = GetImageAvailableSem(),
				.deviceMask = 1,
			};

			const vk::Result res = m_Device.acquireNextImage2KHR(&info, &m_ImageIndex);

			if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR)
			{
				VULKAN_RHI_PANIC("Failed to acquire swapchain element! Error code {}", vk::to_string(res));
			}
		}

		void VulkanRHISwapChain::MakeBackBufferPresentable(RHICommandList::Ptr commandList)
		{
			VulkanRHICommandList* commandBuffer = reinterpret_cast<VulkanRHICommandList*>(commandList.Get());

			commandBuffer->GetCurrentBuffer().endRendering();

			commandBuffer->SetTextureState(
				m_CurrentSwapChain.elements[m_ImageIndex].Get(),
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::PipelineStageFlagBits2::eBottomOfPipe,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::AccessFlagBits2::eNone,
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::ImageLayout::ePresentSrcKHR,
				vk::ImageAspectFlagBits::eColor);

			m_Queue->SetSyncObjects(
				{
					.available = GetImageAvailableSem(),
					.commandsComplete = m_Fence->GetFence()
				});
		}

		void VulkanRHISwapChain::Present()
		{
			VULKAN_RHI_CHECK_RESULT(m_Device.waitForFences({ m_Fence->GetFence() }, vk::True, 1000000000));
			VULKAN_RHI_CHECK_RESULT(m_Device.resetFences({ m_Fence->GetFence() }));;

			vk::PresentInfoKHR presentInfo = 
			{
				.swapchainCount = 1,
				.pSwapchains = &m_CurrentSwapChain.swapchain.get(),
				.pImageIndices = &m_ImageIndex
			};

			vk::Result result = m_Queue->GetQueue().presentKHR(&presentInfo);
			VULKAN_RHI_VERIFYF(result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR,
				"Presentation queue submition failed! Error code {}", vk::to_string(result));

			m_CurrentSemaphoreIndex = (m_CurrentSemaphoreIndex + 1) % m_CurrentSwapChain.imageAvailable.size();
			// NOTE - doing it here since RHI interface does not have separate EndFrame function
			m_WorkCounter.Submit();

			m_CurrentBackBufferIdx = m_WorkCounter.CurrentIndex();
		}

		RenderNativeObject VulkanRHISwapChain::GetNativeObject()
		{
			return RenderNativeObject(&m_CurrentSwapChain.swapchain.get());
		}

		vk::SwapchainKHR VulkanRHISwapChain::GetSwapChain() const 
		{
			return m_CurrentSwapChain.swapchain.get();
		}

		void VulkanRHISwapChain::Resize([[maybe_unused]] RHIDevice::Ptr device, uint32_t width, uint32_t height)
		{
			recreateSwapChain(vk::Extent2D{.width = width, .height = height});
		}

		RHITexture::Ptr VulkanRHISwapChain::GetCurrentBackBuffer()
		{
			return m_CurrentSwapChain.elements[m_ImageIndex];
		}

		void VulkanRHISwapChain::recreateSwapChain(vk::Extent2D resolution)
		{
			const vk::SurfaceCapabilitiesKHR surfaceCapabilities = VulkanUtil::GetCheckedVkValue(
				m_PhysicalDevice.getSurfaceCapabilitiesKHR(m_Surface.GetSurface()));

			const vk::SurfaceFormatKHR format = ChooseSurfaceFormat(m_PhysicalDevice, m_Surface.GetSurface());
			const vk::PresentModeKHR presentMode = ChoosePresentMode(m_PhysicalDevice, m_Surface.GetSurface(), m_UseVsync);
			const vk::Extent2D extent = ChooseSwapChainExtent(surfaceCapabilities, resolution);

			std::uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
			if (surfaceCapabilities.maxImageCount > 0)
			{
				imageCount = std::min(imageCount, surfaceCapabilities.maxImageCount);
			}

			SwapChainData newSwapChain;

			std::size_t imageAvailableSemCount =
				std::max(static_cast<std::size_t>(imageCount), m_WorkCounter.MultiBufferingCount());

			newSwapChain.imageAvailable.resize(imageAvailableSemCount);
			for (std::size_t i = 0; i < newSwapChain.imageAvailable.size(); i++) 
			{
				newSwapChain.imageAvailable[i] =
					VulkanUtil::GetCheckedVkValue(m_Device.createSemaphoreUnique(vk::SemaphoreCreateInfo{}));
			}

			{
				vk::SwapchainCreateInfoKHR createInfo = 
				{
					.surface = m_Surface.GetSurface(),
					.minImageCount = imageCount,
					.imageFormat = format.format,
					.imageColorSpace = format.colorSpace,
					.imageExtent = extent,
					.imageArrayLayers = 1,
					.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
					.imageSharingMode = vk::SharingMode::eExclusive,
					.queueFamilyIndexCount = 1,
					.pQueueFamilyIndices = &m_QueueFamily,
					.preTransform = surfaceCapabilities.currentTransform,
					.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
					.presentMode = presentMode,
					.clipped = vk::True,
					.oldSwapchain = m_CurrentSwapChain.swapchain.get()
				};

				newSwapChain.swapchain = VulkanUtil::GetCheckedVkValue(m_Device.createSwapchainKHRUnique(createInfo));
			}

			newSwapChain.format = format.format;
			newSwapChain.extent = extent;

			std::vector<vk::Image> images =
				VulkanUtil::GetCheckedVkValue(m_Device.getSwapchainImagesKHR(newSwapChain.swapchain.get()));

			newSwapChain.elements.reserve(images.size());

			for (std::size_t i = 0; i < images.size(); i++) 
			{
				vk::ImageViewCreateInfo createInfo = 
				{
					.image = images[i],
					.viewType = vk::ImageViewType::e2D,
					.format = format.format,
					.components = vk::ComponentMapping{},
					.subresourceRange = vk::ImageSubresourceRange
					{
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.baseMipLevel = 0,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1
					}
				};

				newSwapChain.elements.emplace_back(new VulkanRHITexture(RHITexture::Description
					{
						.Dimension = RHITexture::Dimensions::Two,
						// NOTE - circumventing check for initial VulkanRHIContext::Resize()
						// as vulkan gives full window resolution images for swapchain
						.Width = !m_Initialized ? 1 : extent.width,
						.Height = !m_Initialized ? 1 : extent.height,
						.MipLevels = 1,
						.Format = ResourceFormat::BGRA8_UNORM,
						.Flags = RHITexture::UsageFlags::RenderTarget
					},
					std::move(images[i]),
					std::move(VulkanUtil::GetCheckedVkValue(m_Device.createImageViewUnique(createInfo)))));
			}

			m_CurrentSwapChain = std::move(newSwapChain);

			m_Initialized = true;
		}

		vk::Semaphore& VulkanRHISwapChain::GetImageAvailableSem()
		{
			return m_CurrentSwapChain.imageAvailable[m_CurrentSemaphoreIndex].get();
		}
	}
}