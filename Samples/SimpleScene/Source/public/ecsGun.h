#pragma once

#include <flecs.h>

struct ReloadTime {
	float currentValue;
	float maxValue;
};

struct BetweenShotsTime {
	float currentValue;
	float maxValue;
};

struct DisappearenceTime {
	float value;
	bool started;
};

struct AmmoCount {
	int currentValue;
	int maxValue;
};

struct Timer {
	float value;
};

struct Projectile {
};

struct PlayerGun {
};

struct ProjectileCollider {
};

struct ObjectCollider {
};

void RegisterEcsGunSystems(flecs::world& world);