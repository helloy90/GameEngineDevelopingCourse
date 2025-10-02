#include <Camera.h>
#include <DefaultGeometry.h>
#include <Game.h>
#include <GameObject.h>
#include <Input/InputHandler.h>

#include <random>

namespace GameEngine
{
	Game::Game(
		std::function<bool()> PlatformLoopFunc
	) :
		PlatformLoop(PlatformLoopFunc)
	{
		Core::g_MainCamera = new Core::Camera();
		Core::g_MainCamera->SetPosition(Math::Vector3f(0.0f, 6.0f, -6.0f));
		Core::g_MainCamera->SetViewDir(Math::Vector3f(0.0f, -6.0f, 6.0f).Normalized());

		m_renderThread = std::make_unique<Render::RenderThread>();

		std::random_device device;
		std::mt19937 generator(device());
		std::uniform_int_distribution<> distribution(0, 2);

		int objectsAmount = 100;
		int objectsAmountSqrt = static_cast<int>(std::round(std::sqrt(objectsAmount)));
		int spacing = 5;

		// How many objects do we want to create
		for (int i = 0; i < objectsAmount; ++i)
		{
			GameObject::ControllerType controllerType = static_cast<GameObject::ControllerType>(distribution(generator));

			m_Objects.push_back(new GameObject(controllerType));
			Render::RenderObject** renderObject = m_Objects.back()->GetRenderObjectRef();
			m_renderThread->EnqueueCommand(Render::ERC::CreateRenderObject, RenderCore::DefaultGeometry::Cube(), renderObject);
		}

		for (int i = 0; i < objectsAmount; i++) {
			m_Objects[i]->SetPosition(
				Math::Vector3f((i / objectsAmountSqrt) * spacing, 0, (i % objectsAmountSqrt) * spacing),
				m_renderThread->GetMainFrame()
			);
		}

		Core::g_InputHandler->RegisterCallback("GoForward", [&]() { Core::g_MainCamera->Move(Core::g_MainCamera->GetViewDir()); });
		Core::g_InputHandler->RegisterCallback("GoBack", [&]() { Core::g_MainCamera->Move(-Core::g_MainCamera->GetViewDir()); });
		Core::g_InputHandler->RegisterCallback("GoRight", [&]() { Core::g_MainCamera->Move(Core::g_MainCamera->GetRightDir()); });
		Core::g_InputHandler->RegisterCallback("GoLeft", [&]() { Core::g_MainCamera->Move(-Core::g_MainCamera->GetRightDir()); });

		Core::g_InputHandler->RegisterCallback("MoveBlocksLeft", [&]() { keyDirection = Math::Vector3f(-1, 0, 0); });
		Core::g_InputHandler->RegisterCallback("MoveBlocksRight", [&]() { keyDirection = Math::Vector3f(1, 0, 0); });
		Core::g_InputHandler->RegisterCallback("StopBlocks", [&]() { keyDirection = Math::Vector3f(0, 0, 0); });
	}

	void Game::Run()
	{
		assert(PlatformLoop != nullptr);

		m_GameTimer.Reset();

		bool quit = false;
		while (!quit)
		{
			m_GameTimer.Tick();
			float dt = m_GameTimer.GetDeltaTime();

			Core::g_MainWindowsApplication->Update();
			Core::g_InputHandler->Update();
			Core::g_MainCamera->Update(dt);

			Update(dt);

			m_renderThread->OnEndFrame();

			// The most common idea for such a loop is that it returns false when quit is required, or true otherwise
			quit = !PlatformLoop();
		}
	}

	void Game::Update(float dt)
	{
		for (int i = 0; i < m_Objects.size(); ++i)
		{
			m_Objects[i]->Move(dt, keyDirection, m_renderThread->GetMainFrame());
		}
	}
}