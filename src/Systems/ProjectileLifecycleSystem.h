#pragma once

#include "../ECS/ECS.h"
#include "../Components/ProjectileComponent.h"

class ProjectileLifecycleSystem: public System {
	public:
		ProjectileLifecycleSystem() {
			RequireComponent<ProjectileComponent>();
		}

		void Update() {
			for (auto entity: GetSystemEntities()) {
				auto projectile = entity.GetComponent<ProjectileComponent>();

				// TODO kill projectiles after reach duration limit
				if (SDL_GetTicks() - projectile.startTime >= projectile.duration) {
					entity.Kill();
				}
			}
		}
};