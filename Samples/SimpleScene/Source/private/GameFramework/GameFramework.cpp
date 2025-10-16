// This define is essential for the scripts to expose the ECS components to lua syntax
// There is a task to rework this behavior
#define GAME_FRAMEWORK

#include <Camera.h>
#include <DefaultGeometry.h>
#include <ecsControl.h>
#include <ecsMesh.h>
#include <ecsPhys.h>
#include <ecsGun.h>
#include <ECS/ecsSystems.h>
#include <GameFramework/GameFramework.h>
#include <Input/Controller.h>
#include <RenderObject.h>
#include <flecs.h>

#include <random>

using namespace GameEngine;

void GameFramework::Init()
{
	RegisterComponents();
	RegisterSystems();

	flecs::entity cubeControl = m_World.entity()
		.set(Position{ -2.f, 0.f, 0.f })
		.set(Velocity{ 0.f, 0.f, 0.f })
		.set(Speed{ 10.f })
		.set(FrictionAmount{ 0.9f })
		.set(JumpSpeed{ 10.f })
		.set(Gravity{ 0.f, -9.8065f, 0.f })
		.set(BouncePlane{ 0.f, 1.f, 0.f, 5.f })
		.set(Bounciness{ 0.3f })
		.set(EntitySystem::ECS::GeometryPtr{ RenderCore::DefaultGeometry::Cube() })
		.set(EntitySystem::ECS::RenderObjectPtr{ new Render::RenderObject() })
		.set(ControllerPtr{ new Core::Controller(Core::g_FileSystem->GetConfigPath("Input_default.ini")) });

	flecs::entity cubeMoving = m_World.entity()
		.set(Position{ 2.f, 0.f, 0.f })
		.set(Velocity{ 0.f, 3.f, 0.f })
		.set(Gravity{ 0.f, -9.8065f, 0.f })
		.set(BouncePlane{ 0.f, 1.f, 0.f, 5.f })
		.set(Bounciness{ 1.f })
		.set(EntitySystem::ECS::GeometryPtr{ RenderCore::DefaultGeometry::Cube() })
		.set(EntitySystem::ECS::RenderObjectPtr{ new Render::RenderObject() });

	int destructCubesAmount = 10;

	std::random_device device;
	std::mt19937 engine(device());
	std::uniform_real_distribution<float> dist(5, 9);

	for (int i = 0; i < destructCubesAmount; i++) {
		flecs::entity destructCube = m_World.entity()
			.set(Position{ -15.f, 6.f, static_cast<float>(i - destructCubesAmount / 2) * 5.0f })
			.set(Velocity{ 0.f, 0.f, 0.f })
			.set(DestructTimer{ dist(engine) })
			.set(EntitySystem::ECS::GeometryPtr{ RenderCore::DefaultGeometry::Cube() })
			.set(EntitySystem::ECS::RenderObjectPtr{ new Render::RenderObject() });
	}

	int interactableCubesAmount = 5;

	for (int i = 0; i < interactableCubesAmount; i++) {
		flecs::entity cubeInteracting = m_World.entity()
			.set(Position{ 10.f, 10.f, static_cast<float>(i - interactableCubesAmount / 2) * 5.0f })
			.set(Velocity{ 0.f, 0.f, 0.f })
			.set(ObjectCollider{false})
			.set(BouncePlane{ 0.f, 1.f, 0.f, 5.f })
			.set(Bounciness{ 0.3f })
			.set(FrictionAmount{ 0.9f })
			.set(EntitySystem::ECS::GeometryPtr{ RenderCore::DefaultGeometry::Cube() })
			.set(EntitySystem::ECS::RenderObjectPtr{ new Render::RenderObject() });
	}

	flecs::entity gun = m_World.entity()
		.add<PlayerGun>()
		.set(BetweenShotsTime{ 0.5f, 0.5f })
		.set(ControllerPtr{ new Core::Controller(Core::g_FileSystem->GetConfigPath("Input_default.ini")) });

	flecs::entity camera = m_World.entity()
		.set(Position{ 0.0f, 12.0f, -10.0f })
		.set(Speed{ 10.f })
		.set(CameraPtr{ Core::g_MainCamera })
		.set(ControllerPtr{ new Core::Controller(Core::g_FileSystem->GetConfigPath("Input_default.ini")) });
}

void GameFramework::RegisterComponents()
{
	// Exposing these components for the lua system
	ECS_META_COMPONENT(m_World, Position);
	ECS_META_COMPONENT(m_World, Velocity);
	ECS_META_COMPONENT(m_World, Gravity);
	ECS_META_COMPONENT(m_World, BouncePlane);
	ECS_META_COMPONENT(m_World, Bounciness);
	ECS_META_COMPONENT(m_World, ShiverAmount);
	ECS_META_COMPONENT(m_World, FrictionAmount);
	ECS_META_COMPONENT(m_World, Speed);

	ECS_META_COMPONENT(m_World, DestructTimer);
	ECS_META_COMPONENT(m_World, BetweenShotsTime);
	ECS_META_COMPONENT(m_World, ProjectileCollider);
	ECS_META_COMPONENT(m_World, ObjectCollider);
	ECS_META_COMPONENT(m_World, Projectile);
	ECS_META_COMPONENT(m_World, PlayerGun);
}

void GameFramework::RegisterSystems()
{
	RegisterEcsMeshSystems(m_World);
	RegisterEcsControlSystems(m_World);
	RegisterEcsGunSystems(m_World);
}

void GameFramework::Update(float dt)
{

}