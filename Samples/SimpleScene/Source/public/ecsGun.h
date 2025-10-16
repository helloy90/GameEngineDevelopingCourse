#pragma once

#include <flecs.h>

// This define is essential for the scripts to expose the ECS components to lua syntax
// There is a task to rework this behavior
#undef ECS_META_IMPL
#ifndef GAME_FRAMEWORK
#define ECS_META_IMPL EXTERN // Ensure meta symbols are only defined once
#endif

ECS_STRUCT(DestructTimer,
	{
		float value;
	});

ECS_STRUCT(BetweenShotsTime,
	{
		float currentValue;
		float maxValue;
	});

// unused param in struct because no-member structs are not allowed
ECS_STRUCT(ProjectileCollider,
	{
		bool unused;
	});

ECS_STRUCT(ObjectCollider,
	{
		bool unused;
	});

ECS_STRUCT(Projectile,
	{
		float unused;
	});

ECS_STRUCT(PlayerGun,
	{
		float unused;
	});

void RegisterEcsGunSystems(flecs::world& world);