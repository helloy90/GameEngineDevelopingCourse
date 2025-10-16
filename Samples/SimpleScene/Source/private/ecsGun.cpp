#include "ecsGun.h"
#include "ecsControl.h"
#include "ecsMesh.h"
#include "ecsPhys.h"
#include <ECS/ecsSystems.h>
#include <Input/Controller.h>
#include <Camera.h>
#include <Vector.h>
#include <RenderObject.h>
#include <DefaultGeometry.h>

using namespace GameEngine;

void RegisterEcsGunSystems(flecs::world& world)
{
	world.system<BetweenShotsTime, const ControllerPtr>()
		.each([&](BetweenShotsTime& between_shots_time, const ControllerPtr& controller)
			{
				if (between_shots_time.currentValue < between_shots_time.maxValue) {
					between_shots_time.currentValue += world.delta_time();
					return;
				}

				if (controller.ptr->IsPressed("Shoot")) {

					Math::Vector3f cameraPos = Core::g_MainCamera->GetPosition();
					Math::Vector3f cameraDir = Core::g_MainCamera->GetViewDir();
					float speed = 50.0f;

					flecs::entity projectile = world.entity()
						.set(Position{ cameraPos.x, cameraPos.y, cameraPos.z })
						.set(Velocity{ cameraDir.x * speed, cameraDir.y * speed, cameraDir.z * speed })
						.set(Gravity{ 0.f, -9.8065f, 0.f })
						.set(BouncePlane{ 0.f, 1.f, 0.f, 5.f })
						.set(Bounciness{ 0.3f })
						.set(FrictionAmount{ 1.0f })
						.set(EntitySystem::ECS::GeometryPtr{ RenderCore::DefaultGeometry::Cube() })
						.set(EntitySystem::ECS::RenderObjectPtr{ new Render::RenderObject() })
						.add<Projectile>()
						.set(ProjectileCollider{false});

					between_shots_time.currentValue = 0.0f;
				}
			});
}
