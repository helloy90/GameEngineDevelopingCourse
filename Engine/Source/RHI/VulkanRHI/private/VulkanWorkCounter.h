#pragma once

namespace GameEngine
{
	namespace Render::HAL
	{
		class VulkanWorkCounter final 
		{
		private:
			// NOTE - see VulkanRHISwapChain::Present() for reasons
			friend class VulkanRHISwapChain;
		public:
			VulkanWorkCounter() = delete;

			VulkanWorkCounter(std::size_t inflightFrames) 
				: m_CurrentIndex(0)
				, m_InflightFrames(inflightFrames)
			{
			}

		public:
			std::size_t CurrentIndex() const 
			{
				return m_CurrentIndex;
			}

			std::size_t MultiBufferingCount() const 
			{
				return m_InflightFrames;
			}

		private:
			void Submit() 
			{
				m_CurrentIndex = (m_CurrentIndex + 1) % m_InflightFrames;
			}

		private:
			std::size_t m_CurrentIndex;
			// NOTE - initialized with just RenderCore::g_FrameBufferCount for now
			const std::size_t m_InflightFrames;
		};
	}
}