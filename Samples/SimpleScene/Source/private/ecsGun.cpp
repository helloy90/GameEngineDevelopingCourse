#include "ecsGun.h"
#include "ecsControl.h"
#include "ecsPhys.h"
#include "ecsMesh.h"
#include <Input/Controller.h>
#include <Camera.h>
#include <Vector.h>
#include <RenderObject.h>
#include <DefaultGeometry.h>

using namespace GameEngine;

namespace {
	inline void removeEntity(flecs::entity e) {
		e.remove<Velocity>()
			.remove<Gravity>()
			.remove<BouncePlane>()
			.remove<Bounciness>()
			.remove<FrictionAmount>()
			.remove<DisappearenceTime>()
			.remove<Projectile>()
			.remove<ObjectCollider>()
			.remove<ProjectileCollider>();
	}
}

void RegisterEcsGunSystems(flecs::world& world)
{
	world.system<ReloadTime, BetweenShotsTime, AmmoCount, const ControllerPtr>()
		.each([&](ReloadTime& reload_time, BetweenShotsTime& between_shots_time, AmmoCount& ammo_count, const ControllerPtr& controller)
			{
				if (between_shots_time.currentValue < between_shots_time.maxValue) {
					between_shots_time.currentValue += world.delta_time();
					return;
				}

				if (reload_time.currentValue < reload_time.maxValue) {
					reload_time.currentValue += world.delta_time();
					if (reload_time.currentValue >= reload_time.maxValue) {
						ammo_count.currentValue = ammo_count.maxValue;
					}
					return;
				}

				if (controller.ptr->IsPressed("Shoot")) {

					flecs::entity projectile = world.entity()
						.set(Position{ Core::g_MainCamera->GetPosition() })
						.set(Velocity{ Core::g_MainCamera->GetViewDir() * 50.0f })
						.set(Gravity{ Math::Vector3f(0.f, -9.8065f, 0.f) })
						.set(BouncePlane{ Math::Vector4f(0.f, 1.f, 0.f, 5.f) })
						.set(Bounciness{ 0.3f })
						.set(FrictionAmount{ 1.0f })
						.set(DisappearenceTime{ 5.0f, false })
						.set(GeometryPtr{ RenderCore::DefaultGeometry::Cube() })
						.set(RenderObjectPtr{ new Render::RenderObject() })
						.add<Projectile>()
						.add<ProjectileCollider>();

					ammo_count.currentValue--;
					if (ammo_count.currentValue <= 0) {
						reload_time.currentValue = 0.0f;
					}
					between_shots_time.currentValue = 0.0f;
				}
			});


	world.system<DisappearenceTime, Projectile, BouncePlane*, Position*>()
		.each([&](flecs::entity e, DisappearenceTime& time, Projectile, BouncePlane* plane, Position* pos) {

		if (plane && pos && !time.started)
		{
			constexpr float planeEpsilon = 0.1f;
			if (plane->value.x * pos->value.x + plane->value.y * pos->value.y + plane->value.z * pos->value.z < plane->value.w + planeEpsilon)
			{
				time.started = true;
				return;
			}
		}

		time.value -= world.delta_time();

		if (time.value <= 0.0f) {
			if (pos) {
				pos->value = Math::Vector3f(3000.0f, -3000.0f, 3000.0f);
				removeEntity(e);
			}
		}
			});


	world.system<ProjectileCollider, Projectile, Position>()
		.each([&](ProjectileCollider, Projectile, Position& projectile_position) {

		world.each([&](flecs::entity object_e, ObjectCollider, Position& object_position) {
			Math::Vector3f toProjectile = projectile_position.value - object_position.value;
			if (toProjectile.GetLength() > 1.0f) {
				return;
			}
			object_position.value = Math::Vector3f(3000.0f, -3000.0f, 3000.0f);
			removeEntity(object_e);

			world.each([&](PlayerGun, AmmoCount& ammo_count, ReloadTime& time) {
				time.currentValue = time.maxValue;
				ammo_count.currentValue += 3;
			});

		});

	});
}
