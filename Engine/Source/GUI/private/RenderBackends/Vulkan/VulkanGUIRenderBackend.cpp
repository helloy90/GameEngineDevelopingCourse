#include "VulkanGUIRenderBackend.h"

#include "VulkanRHIContext.h"
#include "VulkanRHICore.h"

#include "RenderCore.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

namespace GameEngine::GUI
{
	extern Render::HAL::RHIContext::Ptr g_RHIContext;

  std::optional<vk::DescriptorPool> VulkanRenderBackend::m_DescriptorPool = std::nullopt;

  PFN_vkVoidFunction vulkanLoaderFunction(const char* function_name, void*)
  {
    return VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(
      *(VkInstance*)g_RHIContext->GetFactory()->GetNativeObject(), function_name);
  }

	void VulkanRenderBackend::Init(Render::HAL::RHIContext::Ptr rhiContext) {
	g_RHIContext = rhiContext;

	Render::HAL::VulkanRHIContext* vkRHI = reinterpret_cast<Render::HAL::VulkanRHIContext*>(g_RHIContext.get());

    std::array descriptorTypes = {
      vk::DescriptorPoolSize{vk::DescriptorType::eSampler, 1000},
      vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 1000},
      vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 1000},
      vk::DescriptorPoolSize{vk::DescriptorType::eStorageImage, 1000},
      vk::DescriptorPoolSize{vk::DescriptorType::eUniformTexelBuffer, 1000},
      vk::DescriptorPoolSize{vk::DescriptorType::eStorageTexelBuffer, 1000},
      vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 1000},
      vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, 1000},
      vk::DescriptorPoolSize{vk::DescriptorType::eUniformBufferDynamic, 1000},
      vk::DescriptorPoolSize{vk::DescriptorType::eStorageBufferDynamic, 1000},
      vk::DescriptorPoolSize{vk::DescriptorType::eInputAttachment, 100} };

    vk::DescriptorPoolCreateInfo createInfo = {
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = static_cast<uint32_t>(descriptorTypes.size() * 1000),
      .poolSizeCount = static_cast<uint32_t>(descriptorTypes.size()),
      .pPoolSizes = descriptorTypes.data()
    };

    vk::Device* devicePtr = (vk::Device*)vkRHI->GetDevice()->GetNativeObject();

    vk::ResultValue resVal = devicePtr->createDescriptorPool(createInfo);

    if (resVal.result != vk::Result::eSuccess) {
      assert(false && "failed to create descriptor pool for ImGui!");
    }

    m_DescriptorPool.emplace(std::move(resVal.value));

    std::array targetFormats{ VK_FORMAT_B8G8R8A8_UNORM };
    ImGui_ImplVulkan_InitInfo initInfo = {
      .Instance = *(VkInstance*)g_RHIContext->GetFactory()->GetNativeObject(),
      .PhysicalDevice = static_cast<VkPhysicalDevice>(vkRHI->GetPhysicalDevice()),
      .Device = *(VkDevice*)g_RHIContext->GetDevice()->GetNativeObject(),
      .QueueFamily = vkRHI->GetQueueIdx(),
      .Queue = *(VkQueue*)g_RHIContext->GetCommandQueue()->GetNativeObject(),
      .DescriptorPool = m_DescriptorPool.value(),
      .RenderPass = VK_NULL_HANDLE,
      .MinImageCount = 2,
      .ImageCount =
        std::max(static_cast<uint32_t>(RenderCore::g_FrameBufferCount), uint32_t{2}),
      .MSAASamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
      .PipelineCache = VK_NULL_HANDLE,
      .Subpass = 0,
      .DescriptorPoolSize = 0,
      .UseDynamicRendering = true,
      .PipelineRenderingCreateInfo =
        VkPipelineRenderingCreateInfoKHR{
          .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
          .pNext = nullptr,
          .viewMask = 0,
          .colorAttachmentCount = static_cast<std::uint32_t>(targetFormats.size()),
          .pColorAttachmentFormats = targetFormats.data(),
          .depthAttachmentFormat = {},
          .stencilAttachmentFormat = {},
        },
      .Allocator = nullptr,
      .CheckVkResultFn = nullptr,
      .MinAllocationSize = 0,
    };

    ImGui_ImplVulkan_LoadFunctions(&vulkanLoaderFunction);
    ImGui_ImplVulkan_Init(&initInfo);

	  ImGui_ImplVulkan_CreateFontsTexture();
	}

	void VulkanRenderBackend::Render(ImDrawData* drawData) {
		vk::CommandBuffer* buffer = (vk::CommandBuffer*)g_RHIContext->GetCommandList()->GetNativeObject();
		ImGui_ImplVulkan_RenderDrawData(drawData, *buffer);
	}

	void VulkanRenderBackend::NewFrame() {
		ImGui_ImplVulkan_NewFrame();
	}
}