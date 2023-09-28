#pragma once

#include <SDL2/SDL.h>
#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/ProjectileComponent.h"

class ProjectileEmitSystem: public System {
	public:
		ProjectileEmitSystem() {
			RequireComponent<ProjectileEmitterComponent>();
			RequireComponent<TransformComponent>();
		}

		void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
			eventBus->SubscribeToEvent<KeyPressedEvent>(
				this, &ProjectileEmitSystem::OnKeyPressed
			);
		}

		void OnKeyPressed(KeyPressedEvent& event) {
			if (event.symbol == SDLK_SPACE) {
				for (auto entity: GetSystemEntities()) {
					if (entity.HasComponent<CameraFollowComponent>()) {
						const auto emitter = entity.GetComponent<ProjectileEmitterComponent>();
						const auto transform = entity.GetComponent<TransformComponent>();
						const auto rigidbody = entity.GetComponent<RigidBodyComponent>();

						glm::vec2 projectilePosition = transform.position;
						if (entity.HasComponent<SpriteComponent>()) {
							auto sprite = entity.GetComponent<SpriteComponent>();
							projectilePosition.x += transform.scale.x * sprite.width / 2;
							projectilePosition.y += transform.scale.y * sprite.height / 2;
						}

						glm::vec2 projectileVelocity = emitter.projectileVelocity;
						if (rigidbody.velocity.x != 0 ) {
							projectileVelocity.x = projectileVelocity.x * (rigidbody.velocity.x / std::abs(rigidbody.velocity.x)) + rigidbody.velocity.x;
							projectileVelocity.y = 0.0;
						}
						if (rigidbody.velocity.y != 0 )  {
							projectileVelocity.y = projectileVelocity.y * (rigidbody.velocity.y / std::abs(rigidbody.velocity.y)) + rigidbody.velocity.y;
							projectileVelocity.x = 0.0;
						}

						Entity projectile = entity.registry->CreateEntity();
						projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(1.0, 1.0), 0);
						projectile.AddComponent<RigidBodyComponent>(projectileVelocity);
						projectile.AddComponent<SpriteComponent>("bullet-image", 4, 4, 4);
						projectile.AddComponent<BoxColliderComponent>(4, 4);
						projectile.AddComponent<ProjectileComponent>(
							emitter.isFriendly, 
							emitter.hitPercentDamage, 
							emitter.projectileDuration
						);
					}
				}
			}
		}

		void Update() {
			for (auto entity: GetSystemEntities()) {
				auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
				auto transform = entity.GetComponent<TransformComponent>();

				if (projectileEmitter.repeatFrequency == 0 ) {
					continue;
				}

				if (SDL_GetTicks() - projectileEmitter.lastEmissionTime > projectileEmitter.repeatFrequency) {
					glm::vec2 projectilePosition = transform.position;

					if (entity.HasComponent<SpriteComponent>()) {
						auto sprite = entity.GetComponent<SpriteComponent>();
						projectilePosition.x += transform.scale.x * sprite.width / 2;
						projectilePosition.y += transform.scale.y * sprite.height / 2;
					}

					Entity projectile = entity.registry->CreateEntity();
					projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(1.0, 1.0), transform.rotation);
					projectile.AddComponent<RigidBodyComponent>(projectileEmitter.projectileVelocity);
					projectile.AddComponent<SpriteComponent>("bullet-image", 4, 4, 4);
					projectile.AddComponent<BoxColliderComponent>(4, 4);
					projectile.AddComponent<ProjectileComponent>(
						projectileEmitter.isFriendly, 
						projectileEmitter.hitPercentDamage, 
						projectileEmitter.projectileDuration
					);

					projectileEmitter.lastEmissionTime = SDL_GetTicks();
				}
			}
		}
};