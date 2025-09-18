#include <RenderEngine.h>
#include <RHIHelper.h>
#include <Mesh.h>

namespace GameEngine::Render
{
	RenderEngine::RenderEngine()
	{
		m_rhi = HAL::RHIHelper::CreateRHI("D3D12");
		m_rhi->Init();

		//RenderObject::Ptr box = std::make_shared<RenderObject>();
		//m_RenderObjects.push_back(box);
		//box->m_mesh = m_rhi->CreateBoxMesh();
		//box->m_material = m_rhi->GetMaterial(box->m_mesh->GetName());
		RenderObject::Ptr pyramid = std::make_shared<RenderObject>();
		m_RenderObjects.push_back(pyramid);
		pyramid->m_mesh = m_rhi->CreatePyramidMesh();
		pyramid->m_material = m_rhi->GetMaterial(pyramid->m_mesh->GetName());
		
		m_rhi->ExecuteCommandLists();
		m_rhi->Flush();
	}

	void RenderEngine::Update()
	{
		m_rhi->Update(m_RenderObjects[0]->m_mesh, m_RenderObjects[0]->m_material);

		m_rhi->Flush();
	}

	void RenderEngine::OnResize(uint16_t width, uint16_t height)
	{
		m_swapChainWidth = width;
		m_swapChainHeight = height;
		m_rhi->OnResize();
	}
}